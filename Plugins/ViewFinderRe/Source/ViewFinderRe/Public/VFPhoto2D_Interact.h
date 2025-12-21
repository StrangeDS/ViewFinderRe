// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VFPhoto2DSteppable.h"
#include "VFInteractInterface.h"
#include "VFPhoto2D_Interact.generated.h"

class UUserWidget;

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERRE_API AVFPhoto2D_Interact : public AVFPhoto2DSteppable, public IVFInteractInterface
{
	GENERATED_BODY()

public:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TSubclassOf<UUserWidget> AimingHintUMGClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<UUserWidget> AimingHintUMG;

public: // implements IVFInteractInterface
	virtual bool StartAiming_Implementation(APlayerController *Controller) override;

	virtual bool EndAiming_Implementation(APlayerController *Controller) override;

	virtual bool Interact_Implementation(APlayerController *Controller) override;
};