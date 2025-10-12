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
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool Visible = false;

	FVFTransCompInfo() {}

	FVFTransCompInfo(USceneComponent *Comp)
		: Component(Comp),
		  Transform(Comp->GetComponentTransform()),
		  Velocity(Comp->GetComponentVelocity()),
		  Visible(Comp->IsVisible()) {};

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

// 对Actor瞬移(SetActorLocation())的回退, 效果不佳, 会认为是从不动后到瞬移后的长时间位移.
// VFCharacter对瞬移的回退效果较好, 可以考虑进行改进. (懒得改了)
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
	考虑存在有USceneComponent作为根节点的Actor
	所以这里依然使用USceneComponent作为基本元素
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
