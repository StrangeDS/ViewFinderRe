// Copyright 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFStepsRecordInterface.h"
#include "VFTimeRewindComponent.generated.h"

class UVFStepsRecorderWorldSubsystem;
class APawn;

/** A pawn pose and velocity sampled at the rewind subsystem's time. */
USTRUCT(BlueprintType)
struct FVFPawnRewindSample
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
    FRotator Rotator = FRotator::ZeroRotator;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
    float Time = 0.f;

    FVFPawnRewindSample() = default;

    FVFPawnRewindSample(APawn *Pawn, float TimeIn);

    FVFPawnRewindSample(const FVFPawnRewindSample &Other, float TimeIn);

    bool operator==(const FVFPawnRewindSample &Other) const;
};

/** Records pawn movement through the world rewind subsystem without requiring AVFCharacter. */
UCLASS(Blueprintable, ClassGroup = (ViewFinder), meta = (BlueprintSpawnableComponent))
class VIEWFINDERRE_API UVFTimeRewindComponent : public UActorComponent, public IVFStepsRecordInterface
{
    GENERATED_BODY()

public:
    UVFTimeRewindComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual void TickForward_Implementation(float Time) override;

    virtual void TickBackward_Implementation(float Time) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
    TArray<FVFPawnRewindSample> Steps;

private:
    /** Cached pointer to the world subsystem, lazy-initialized. */
    UPROPERTY()
    mutable TObjectPtr<UVFStepsRecorderWorldSubsystem> StepsRecorder = nullptr;

    UVFStepsRecorderWorldSubsystem *GetStepsRecorder() const;
};
