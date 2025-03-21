#pragma once

#include "CoreMinimal.h"
#include "VFPhotoCatcherSteppable.h"

#include "GameFramework/Pawn.h"
#include "VFInteractInterface.h"

#include "VFPhotoCatcher_Interact.generated.h"

class UUserWidget;

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERRE_API AVFPhotoCatcher_Interact : public AVFPhotoCatcherSteppable, public IVFInteractInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	bool bInteractingEnabled = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	TEnumAsByte<ECollisionEnabled::Type> CollisionEnabledRecord;

	virtual bool StartAiming_Implementation(APlayerController *Controller) override;

	virtual bool EndAiming_Implementation(APlayerController *Controller) override;

	virtual bool Interact_Implementation(APlayerController *Controller) override;

	virtual bool IsEnabled_Implementation(APlayerController *Controller) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void EnableInteract(bool Enabled);
	virtual void EnableInteract_Implementation(bool Enabled);

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void CloseToPreview();
	virtual void CloseToPreview_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void LeaveFromPreview();
	virtual void LeaveFromPreview_Implementation();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TSubclassOf<UUserWidget> AimingHintUMGClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<UUserWidget> AimingHintUMG;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float TimeOfClose = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float TimeOfLeave = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	class UInputMappingContext *AimingMappingContext;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<APawn> Pawn;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<APlayerController> PlayerController;
};
