#pragma once

#include "CoreMinimal.h"
#include "VFPhotoCatcher_Interact.h"
#include "VFActivatableInterface.h"
#include "VFPhotoCatcher_PickUp.generated.h"

class UInputMappingContext;

class AVFPhotoContainer;

UENUM()
enum class EVFPhotoCatcherPickUpOption : uint8
{
	PickedUp,
	DroppedDown,
	MAX,
};

USTRUCT(BlueprintType)
struct FVFPhotoCatcherPickUpStepInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	EVFPhotoCatcherPickUpOption Option;

	// 根据类型显示对应字段
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (EditCondition = "Option == EVFPhotoCatcherPickUpOption::PickedUp"))
	FTransform Transform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (EditCondition = "Option == EVFPhotoCatcherPickUpOption::DroppedDown"))
	TObjectPtr<USceneComponent> CompAttached;
};

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERRE_API AVFPhotoCatcher_PickUp : public AVFPhotoCatcher_Interact,
												public IVFActivatableInterface
{
	GENERATED_BODY()

public:
	virtual bool Interact_Implementation(APlayerController *Controller) override;

	virtual AVFPhoto2D *TakeAPhoto_Implementation() override;

	virtual void CloseToPreview_Implementation() override;

	virtual void LeaveFromPreview_Implementation() override;

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void PickUp(USceneComponent *ToAttach);
	virtual void PickUp_Implementation(USceneComponent *ToAttach);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void DropDown();
	virtual void DropDown_Implementation();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UInputMappingContext> HoldingMappingContext;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	bool bPickedUp = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	bool bReady = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<AVFPhotoContainer> Container;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void CloseToPreview_Move();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void LeaveFromPreview_Move();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder", meta = (MakeEditWidget))
	FTransform IdleTrans = FTransform(FRotator::ZeroRotator, FVector(100.0f, 20.0f, -20.0f));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder", meta = (MakeEditWidget))
	FTransform PreviewTrans = FTransform(FRotator::ZeroRotator, FVector(17.5f, 0.f, 0.f));

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	FTimerHandle PreviewTimeHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float PreviewMoveInterval = 0.02f;

public: // IVFActivatableInterface
	virtual void Activate_Implementation() override;

	virtual void Deactivate_Implementation() override;

	virtual bool CanActivate_Implementation() override;

	virtual bool IsActive_Implementation() override;

public:
	virtual bool StepBack_Implementation(FVFStepInfo &StepInfo) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TArray<FVFPhotoCatcherPickUpStepInfo> StepInfos;
};