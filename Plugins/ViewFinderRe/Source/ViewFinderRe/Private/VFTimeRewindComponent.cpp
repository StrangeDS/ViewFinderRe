// Copyright 2026. All Rights Reserved.

#include "VFTimeRewindComponent.h"

#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Runtime/Launch/Resources/Version.h"

#include "VFLog.h"
#include "VFStepsRecorderWorldSubsystem.h"

namespace
{
void RestorePawnVelocity(APawn *Pawn, const FVector &Velocity)
{
    USceneComponent *RootComponent = Pawn->GetRootComponent();
    if (RootComponent && RootComponent->IsSimulatingPhysics())
    {
        CastChecked<UPrimitiveComponent>(RootComponent)->SetPhysicsLinearVelocity(Velocity);
    }
    else if (auto *Character = Cast<ACharacter>(Pawn))
    {
        Character->GetCharacterMovement()->Velocity = Velocity;
    }
    else if (auto *Movement = Pawn->GetMovementComponent())
    {
        Movement->Velocity = Velocity;
    }
}
} // namespace

FVFPawnRewindSample::FVFPawnRewindSample(APawn *Pawn, float TimeIn)
    : Location(Pawn->GetActorLocation()), Velocity(Pawn->GetVelocity()), Rotator(Pawn->GetViewRotation()),
      Time(TimeIn)
{
}

FVFPawnRewindSample::FVFPawnRewindSample(const FVFPawnRewindSample &Other, float TimeIn)
    : Location(Other.Location), Velocity(Other.Velocity), Rotator(Other.Rotator), Time(TimeIn)
{
}

bool FVFPawnRewindSample::operator==(const FVFPawnRewindSample &Other) const
{
    if (Location != Other.Location)
    {
        return false;
    }
    if (Velocity != Other.Velocity)
    {
        return false;
    }
    return Rotator == Other.Rotator;
}

UVFTimeRewindComponent::UVFTimeRewindComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

UVFStepsRecorderWorldSubsystem *UVFTimeRewindComponent::GetStepsRecorder() const
{
    if (!StepsRecorder)
    {
        const UWorld *World = GetWorld();
        if (World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE))
        {
            StepsRecorder = World->GetSubsystem<UVFStepsRecorderWorldSubsystem>();
        }
    }
    return StepsRecorder;
}

void UVFTimeRewindComponent::BeginPlay()
{
    Super::BeginPlay();

    if (!GetStepsRecorder())
    {
        VF_LOG(Error, TEXT("%s: invalid StepsRecorder."), __FUNCTIONW__);
        return;
    }
    Steps.Reserve(UVFStepsRecorderWorldSubsystem::GetSizeRecommended());
    StepsRecorder->RegisterTickable(this);
    if (Cast<APawn>(GetOwner()))
    {
        TickForward_Implementation(StepsRecorder->GetTime());
    }
}

void UVFTimeRewindComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (GetStepsRecorder())
    {
        // Registration is deferred until the next recorder tick; destruction can happen first.
        StepsRecorder->TargetsNeedToAdd.Remove(this);
        if (StepsRecorder->IsTickableRegistered(this))
        {
            StepsRecorder->UnregisterTickable(this);
        }
    }
    Super::EndPlay(EndPlayReason);
}

void UVFTimeRewindComponent::TickForward_Implementation(float Time)
{
    APawn *Pawn = Cast<APawn>(GetOwner());
    if (!Pawn)
    {
        return;
    }

    static constexpr float MaxContinuousSampleGap = 0.1f;

    FVFPawnRewindSample Info(Pawn, StepsRecorder->Time);
    if (Steps.IsEmpty())
    {
        Steps.Add(Info);
        return;
    }

    if (Steps.Last() != Info)
    {
        // Keep a stationary interval flat instead of interpolating across a long sampling gap.
        auto LastInfo = Steps.Last();
        if (Time - LastInfo.Time > MaxContinuousSampleGap)
        {
            FVFPawnRewindSample RepeatInfo(LastInfo, StepsRecorder->Time);
            Steps.Add(RepeatInfo);
        }

        Steps.Add(Info);
    }
}

void UVFTimeRewindComponent::TickBackward_Implementation(float Time)
{
    APawn *Pawn = Cast<APawn>(GetOwner());
    if (!Pawn)
    {
        return;
    }

    while (Steps.Num() > 1)
    {
        auto &StepInfo = Steps.Last();
        if (StepInfo.Time < Time)
        {
            break;
        }

        Pawn->SetActorLocation(StepInfo.Location);
        RestorePawnVelocity(Pawn, StepInfo.Velocity);
        if (Pawn->GetController())
        {
            Pawn->GetController()->SetControlRotation(StepInfo.Rotator);
        }

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 5
        Steps.Pop(EAllowShrinking::No);
#else
        Steps.Pop(false);
#endif
    }

    if (Steps.IsEmpty())
    {
        return;
    }
    auto &Step = Steps.Last();
    if (Time < Step.Time)
    {
        return;
    }

    auto Delta =
        StepsRecorder->GetDeltaTime() / FMath::Max(StepsRecorder->GetTime() - Step.Time, SMALL_NUMBER);
    Delta = FMath::Min(Delta, 1.0f);
    Pawn->SetActorLocation(FMath::Lerp(Pawn->GetActorLocation(), Step.Location, Delta));

    auto Velocity = FMath::Lerp(Pawn->GetVelocity(), Step.Velocity, Delta);
    RestorePawnVelocity(Pawn, Velocity);

    if (Pawn->GetController())
    {
        Pawn->GetController()->SetControlRotation(FMath::Lerp(Pawn->GetViewRotation(), Step.Rotator, Delta));
    }
}
