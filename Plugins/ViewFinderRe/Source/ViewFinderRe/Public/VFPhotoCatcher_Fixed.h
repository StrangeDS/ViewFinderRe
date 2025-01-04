#pragma once

#include "CoreMinimal.h"
#include "VFPhotoCatcher_Interact.h"
#include "VFPhotoCatcher_Fixed.generated.h"

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERRE_API AVFPhotoCatcher_Fixed : public AVFPhotoCatcher_Interact
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder", meta = (MakeEditWidget))
	FTransform PhotoSpawnPoint = FTransform::Identity;

	virtual AVFPhoto2D *TakeAPhoto_Implementation() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float TimeOfTakingPhoto = 0.1f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	FTimerHandle TimerHandleOfTakingPhoto;

public:
	virtual bool StartAiming_Implementation(APlayerController *Controller) override;

	virtual bool EndAiming_Implementation(APlayerController *Controller) override;

	virtual bool Interact_Implementation(APlayerController *Controller) override;
};
