// Copyright 2026, StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFStepsRecordInterface.h"
#include "VFStepsRecorderWorldSubsystem.h"
#include "VFTransformRecorderActor.generated.h"

USTRUCT(BlueprintType)
struct FVFTransCompInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<USceneComponent> Component;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	FTransform Transform = FTransform::Identity;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	FVector LinearVelocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	FVector AngularVelocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool Visible = false;

	FVFTransCompInfo() {}

	FVFTransCompInfo(USceneComponent *Comp);

	bool operator==(const FVFTransCompInfo &Other) const;

	bool IsChanged(const FVFTransCompInfo &InfoNew) const;
};

USTRUCT(BlueprintType)
struct FVFTransStepInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float Time = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<FVFTransCompInfo> Infos;

	FVFTransStepInfo() {};

	FVFTransStepInfo(float TimeIn, TArray<FVFTransCompInfo> InfosIn)
		: Time(TimeIn),
		  Infos(InfosIn) {};
};

/*
Rewinding for Actor teleportation (via SetActorLocation()) produces poor results.
This is because the tansform is not resubmitted before teleportation.
It is Considered as a long-distance transition from the last stationary position directly to the teleported location.
VFCharacter handles teleportation rewinding more effectively and can be referenced for improvements.
*/
UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VFSTEPSRECORDER_API AVFTransformRecorderActor : public AActor, public IVFStepsRecordInterface
{
	GENERATED_BODY()

public:
	AVFTransformRecorderActor();

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void AddToRecord(USceneComponent *Component);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void RemoveFromRecord(USceneComponent *Component);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	bool IsBegingRecorded(USceneComponent *Component);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TSubclassOf<UPrimitiveComponent> CompClassToCollect;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ViewFinder")
	void ReCollectComponents();
	virtual void ReCollectComponents_Implementation();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void ClearComponents();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void ClearInfoMap();

protected:
	/*
	Considering Actors with USceneComponent as root nodes:
	USceneComponent is still used as the fundamental element here.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TObjectPtr<USceneComponent>> Components;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TMap<TObjectPtr<USceneComponent>, FVFTransCompInfo> CompInfoMap;

public: // Implements IVFStepsRecordInterface:
	virtual void TickForward_Implementation(float Time) override;

	virtual void TickBackward_Implementation(float Time) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TArray<FVFTransStepInfo> Steps;

	DECLARE_STEPSRECORDER_SUBSYSTEM_ACCESSOR(StepsRecorder);
};