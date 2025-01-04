#pragma once

#include "CoreMinimal.h"
#include "VFPhoto2DSteppable.h"

#include "GameFramework/Pawn.h"
#include "VFInteractInterface.h"

#include "VFPhoto2D_Interact.generated.h"

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERRE_API AVFPhoto2D_Interact : public AVFPhoto2DSteppable, public IVFInteractInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TSubclassOf<class UUserWidget> AimingHintUMGClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<class UUserWidget> AimingHintUMG;

public: // implements IVFInteractInterface
	virtual bool StartAiming_Implementation(APlayerController *Controller) override;

	virtual bool EndAiming_Implementation(APlayerController *Controller) override;

	virtual bool Interact_Implementation(APlayerController *Controller) override;
};
