// Copyright StrangeDS. All Rights Reserved.

#include "VFStepsRecorderWorldSubsystem.h"

#include "UObject/Package.h"
#include "Algo/BinarySearch.h"

#include "VFLog.h"
#include "VFStepsRecordInterface.h"
#include "VFStepsRecorderDeveloperSettings.h"
#include "VFTransformRecorderActor.h"

TStatId UVFStepsRecorderWorldSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UVFStepsRecorderWorldSubsystem, STATGROUP_Tickables);
}

void UVFStepsRecorderWorldSubsystem::OnWorldBeginPlay(UWorld &InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    auto Settings = GetDefault<UVFStepsRecorderDeveloperSettings>();
    TickInterval = Settings->StepsRecorderTickInterval;
    RewindCurFactor = Settings->StepsRecorderRewindFactor;

    Infos.Reserve(10 * GetSizeRecommended());
}

void UVFStepsRecorderWorldSubsystem::Tick(float DeltaTime)
{
    // Prioritize processing changes to TickTargets.
    if (TargetsNeedToAdd.Num() > 0)
    {
        TickTargets.Append(TargetsNeedToAdd);
        TargetsNeedToAdd.Reset();
    }
    else if (TargetsNeedToRemove.Num() > 0)
    {
        TickTargets.RemoveAll(
            [this](
                const TScriptInterface<IVFStepsRecordInterface> &Target)
            { return TargetsNeedToRemove.Contains(Target); });
        TargetsNeedToRemove.Reset();
    }

    // Forward/Reverse tick.
    TimeSinceLastTick += bIsRewinding ? DeltaTime * RewindCurFactor : DeltaTime;
    while (TimeSinceLastTick > TickInterval)
    {
        Time += bIsRewinding ? -TickInterval : TickInterval;
        if (bIsRewinding)
        {
            TickBackward(TickInterval);
        }
        else
        {
            TickForward(TickInterval);
        }
        TimeSinceLastTick -= TickInterval;
        OnTickTime.Broadcast(Time);
    }
}

void UVFStepsRecorderWorldSubsystem::Deinitialize()
{
    for (auto &[Channel, TransformRecorder] : TransformRecorderMap)
    {
        TransformRecorder->Destroy();
    }
    TransformRecorderMap.Empty();

    Super::Deinitialize();
}

void UVFStepsRecorderWorldSubsystem::SubmitStep(UObject *Sender, FVFStepInfo Info)
{
    if (!IsValid(Sender))
    {
        VF_LOG(Error, TEXT("%s: Sender is null or pending kill"), __FUNCTIONW__);
        return;
    }

    if (!Sender->Implements<UVFStepsRecordInterface>())
    {
        VF_LOG(Error, TEXT("%s: invliad Sender(%s) in %s."),
               __FUNCTIONW__,
               *Sender->GetName(),
               *Sender->GetOutermost()->GetName());
        return;
    }

    Info.Sender = Sender;
    if (Info.Time < 0.f)
        Info.Time = Time;

    if (Infos.IsEmpty() || Infos.Last().Time <= Info.Time)
    {
        Infos.Add(Info);
    }
    else
    {
        // Insertion sort, binary search.
        int32 Index = Algo::UpperBound(Infos, Info);
        Infos.Insert(Info, Index);
    }
}

void UVFStepsRecorderWorldSubsystem::TickForward(float DeltaTime)
{
    if (Time >= TIME_MAX)
    {
        Time = TIME_MAX;
        return;
    }

    Time = FMath::Min(Time, TimeOfEnd);

    for (auto It = TickTargets.CreateIterator(); It; ++It)
    {
        auto Target = *It;
        if (!IsValid(Target.GetObject()))
        {
            It.RemoveCurrent();
        }
        else
        {
            IVFStepsRecordInterface::Execute_TickForward(Target.GetObject(), Time);
        }
    }
}

void UVFStepsRecorderWorldSubsystem::TickBackward(float DeltaTime)
{
    // Do not rewind to TIME_MIN.
    if (Time <= TIME_MIN)
    {
        Time = TIME_MIN;
        EndRewinding();
        return;
    }

    // Rewind to TimeOfStart at most.
    if (Time <= TimeOfStart)
        Time = TimeOfStart;

    while (!Infos.IsEmpty())
    {
        auto Info = Infos.Last();
        if (Info.Time < Time)
            break;

        if (!IsValid(Info.Sender))
            continue;

        bool handled = IVFStepsRecordInterface::Execute_StepBack(Info.Sender, Info);
        if (!handled)
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("UVFStepsRecorderWorldSubsystem::TickBackward(): StepInfo(%s) from %s does not get handled"),
                *Info.Info,
                *Info.Sender->GetName());
        }
        Infos.Pop(false);
    }

    for (auto &Target : TickTargets)
    {
        if (IsValid(Target.GetObject()))
            IVFStepsRecordInterface::Execute_TickBackward(Target.GetObject(), Time);
    }

    if (Time <= TimeOfStart)
        EndRewinding();
}

void UVFStepsRecorderWorldSubsystem::RecordTransform(
    USceneComponent *Component, const FString &Channel)
{
    check(IsValid(Component));

    if (Component->Mobility != EComponentMobility::Movable)
    {
        VF_LOG(Error,
               TEXT("Component(%s) on Actor(%s) Mobility is not Movable."),
               *Component->GetName(),
               *Component->GetOuter()->GetName());
        return;
    }

    if (!TransformRecorderMap.Contains(Channel))
    {
        auto TransformRecorder = GetWorld()->SpawnActor<AVFTransformRecorderActor>();
        TransformRecorderMap.Emplace(Channel, TransformRecorder);
    }
    TransformRecorderMap[Channel]->AddToRecord(Component);
}

void UVFStepsRecorderWorldSubsystem::UnrecordTransform(
    USceneComponent *Component, const FString &Channel)
{
    check(IsValid(Component));

    if (Component->Mobility != EComponentMobility::Movable)
    {
        VF_LOG(Error,
               TEXT("Component(%s) on Actor(%s) Mobility is not Movable."),
               *Component->GetName(),
               *Component->GetOuter()->GetName());
        return;
    }

    if (!TransformRecorderMap.Contains(Channel))
    {
        VF_LOG(Error, TEXT("%s invalid channel."), __FUNCTIONW__);
        return;
    }

    if (!TransformRecorderMap[Channel]->IsBegingRecorded(Component))
    {
        VF_LOG(Error, TEXT("%s Comp is not being Recorded."), __FUNCTIONW__);
        return;
    }
    TransformRecorderMap[Channel]->RemoveFromRecord(Component);
}

bool UVFStepsRecorderWorldSubsystem::IsTransformRecorded(USceneComponent *Component, const FString &Channel)
{
    check(IsValid(Component));

    return TransformRecorderMap.Contains(Channel) &&
           TransformRecorderMap[Channel]->IsBegingRecorded(Component);
}

void UVFStepsRecorderWorldSubsystem::RegisterTickable(const TScriptInterface<IVFStepsRecordInterface> &Target)
{
    if (IsValid(Target.GetObject()))
        TargetsNeedToAdd.AddUnique(Target);
}

void UVFStepsRecorderWorldSubsystem::UnregisterTickable(const TScriptInterface<IVFStepsRecordInterface> &Target)
{
    if (ensure(TickTargets.Contains(Target)))
    {
        TargetsNeedToRemove.AddUnique(Target);
    }
}

bool UVFStepsRecorderWorldSubsystem::IsTickableRegistered(const TScriptInterface<IVFStepsRecordInterface> &Target)
{
    return TickTargets.Contains(Target);
}

void UVFStepsRecorderWorldSubsystem::StartRewinding()
{
    if (bIsRewinding)
        return;

    TimeSinceLastTick = 0.f;
    OnStartRewinding.Broadcast(Time);
    bIsRewinding = true;
}

void UVFStepsRecorderWorldSubsystem::EndRewinding()
{
    if (!bIsRewinding)
        return;

    TimeSinceLastTick = 0.f;
    OnEndRewinding.Broadcast(Time);
    bIsRewinding = false;
}

void UVFStepsRecorderWorldSubsystem::SetTimeOfStart(float Start)
{
    if (Start <= TIME_MIN)
        Start = Time;

    TimeOfStart = Start;
    check(TimeOfStart <= TimeOfEnd);
}

void UVFStepsRecorderWorldSubsystem::SetTimeOfEnd(float End)
{
    if (End < 0.f || End >= TIME_MAX)
        End = TIME_MAX - TickInterval;

    TimeOfEnd = End;
    check(TimeOfStart <= TimeOfEnd);
}

float UVFStepsRecorderWorldSubsystem::GetTimeOfStart()
{
    return TimeOfStart;
}

float UVFStepsRecorderWorldSubsystem::GetTimeOfEnd()
{
    return TimeOfEnd;
}

float UVFStepsRecorderWorldSubsystem::GetTimeOfMin()
{
    return TIME_MIN;
}

float UVFStepsRecorderWorldSubsystem::GetTimeOfMax()
{
    return TIME_MAX;
}

int UVFStepsRecorderWorldSubsystem::GetSizeRecommended()
{
    return GetDefault<UVFStepsRecorderDeveloperSettings>()->StepsRecorderSizeRecommended;
}

void UVFStepsRecorderWorldSubsystem::RewindToLastKey()
{
    for (int i = Infos.Num() - 1; i >= 0; i--)
    {
        if (Infos[i].bIsKeyFrame)
        {
            TimeOfRewindingTarget = Infos[i].Time;
            break;
        }
    }

    float TimeSpan = Time - TimeOfRewindingTarget;
    auto Settings = GetDefault<UVFStepsRecorderDeveloperSettings>();
    float TimeOfBackard = TimeSpan / Settings->StepsRecorderRewindFactor;
    TimeOfBackard = FMath::Min(TimeOfBackard, Settings->StepsRecorderTimeOfRewindToLastKey);
    RewindCurFactor = TimeSpan / TimeOfBackard;

    OnTickTime.AddUniqueDynamic(this, &UVFStepsRecorderWorldSubsystem::CheckRewoundToLastKeyPoint);

    StartRewinding();
}

void UVFStepsRecorderWorldSubsystem::CheckRewoundToLastKeyPoint(float TimeCur)
{
    if (TimeCur > TimeOfRewindingTarget)
        return;

    TimeOfRewindingTarget = TimeOfStart;
    RewindCurFactor = GetDefault<UVFStepsRecorderDeveloperSettings>()->StepsRecorderRewindFactor;

    OnTickTime.RemoveDynamic(this, &UVFStepsRecorderWorldSubsystem::CheckRewoundToLastKeyPoint);

    EndRewinding();
}

UVFStepsRecorderWorldSubsystem *UVFStepsRecorderWorldSubsystem::GetStepsRecorder(
    const UObject *WorldContext,
    const EVFStepsRecorderSubsystemCheckMode Mode)
{
    const UWorld *World = WorldContext->GetWorld();
    if (IsValid(World) && World->IsGameWorld())
    {
        auto System = World->GetSubsystem<UVFStepsRecorderWorldSubsystem>();
        switch (Mode)
        {
        case EVFStepsRecorderSubsystemCheckMode::NotRewinding:
            return System->bIsRewinding ? nullptr : System;
        case EVFStepsRecorderSubsystemCheckMode::InGameWorld:
        default:
            return System;
        }
    }

    VF_LOG(Warning, TEXT("%s should only be called in runtime."), __FUNCTIONW__);
    return nullptr;
}