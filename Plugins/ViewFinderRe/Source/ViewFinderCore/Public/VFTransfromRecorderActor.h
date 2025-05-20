#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFStepsRecordInterface.h"
#include "VFStepsRecorderWorldSubsystem.h"
#include "VFTransfromRecorderActor.generated.h"

USTRUCT(BlueprintType)
struct FVFTransCompInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<USceneComponent> Component;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	FTransform Transform = FTransform::Identity;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	FVector Velocity = FVector::ZeroVector;

	FVFTransCompInfo() {}
	FVFTransCompInfo(USceneComponent *Comp) : Component(Comp),
											  Transform(Comp->GetComponentTransform()),
											  Velocity(Comp->GetComponentVelocity()) {};

	bool operator==(const FVFTransCompInfo &Other) const;
};

USTRUCT(BlueprintType)
struct FVFTransStepInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float Time = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<FVFTransCompInfo> Infos;

	FVFTransStepInfo() {};
	FVFTransStepInfo(float TimeIn, TArray<FVFTransCompInfo> InfosIn) : Time(TimeIn),
																	   Infos(InfosIn) {};
};

// 对Actor瞬移(SetActorLocation())的回退, 效果不佳, 会认为是从不动后到瞬移后的长时间位移.
// VFCharacter对瞬移的回退效果较好, 可以考虑进行改进. (懒得改了)
UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFTransfromRecorderActor : public AActor, public IVFStepsRecordInterface
{
	GENERATED_BODY()

public:
	AVFTransfromRecorderActor();

	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void AddToRecord(USceneComponent *Component);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void RemoveFromRecord(USceneComponent *Component);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ViewFinder")
	void ReCollectComponents();
	void ReCollectComponents_Implementation();

protected:
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
