#pragma once

#include "CoreMinimal.h"
#include "VFPhotoCatcher_Interact.h"
#include "VFPhotoCatcher_PickUp.generated.h"

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERRE_API AVFPhotoCatcher_PickUp : public AVFPhotoCatcher_Interact
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
	void PickUp_Implementation(USceneComponent *ToAttach);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void DropDown();
	void DropDown_Implementation();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UInputMappingContext> HoldingMappingContext;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	bool bPickedUp = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	bool bReady = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<class AVFPhotoContainer> Container;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void CloseToPreview_Move();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void LeaveFromPreview_Move();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	FTransform IdleTrans = FTransform(FRotator::ZeroRotator, FVector(50.0f, 30.0f, -20.0f));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	FTransform PreviewTrans = FTransform(FRotator::ZeroRotator, FVector(50.f, 0.f, 0.f));

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	FTimerHandle PreviewTimeHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float PreviewMoveInterval = 0.02f;
};
