#include "VFPhoto2D_Interact.h"

#include "GameFramework/Pawn.h"
#include "Blueprint/UserWidget.h"

#include "VFPhotoContainerInterface.h"

bool AVFPhoto2D_Interact::StartAiming_Implementation(APlayerController *Controller)
{
    if (!AimingHintUMGClass.Get())
        return false;

    if (!AimingHintUMG)
        AimingHintUMG = CreateWidget<UUserWidget>(GetWorld(), AimingHintUMGClass);
    AimingHintUMG->AddToViewport();

    return true;
}

bool AVFPhoto2D_Interact::EndAiming_Implementation(APlayerController *Controller)
{
    if (AimingHintUMG)
        AimingHintUMG->RemoveFromParent();

    return true;
}

bool AVFPhoto2D_Interact::Interact_Implementation(APlayerController *Controller)
{
    auto Pawn = Controller->GetPawn();
    if (Pawn->Implements<UVFPhotoContainerInterface>())
    {
        return IVFPhotoContainerInterface::Execute_TakeIn(Pawn, this);
    }
    return false;
}