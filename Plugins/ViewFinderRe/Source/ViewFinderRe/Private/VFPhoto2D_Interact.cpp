#include "VFPhoto2D_Interact.h"

#include "Blueprint/UserWidget.h"

#include "VFPhotoContainer.h"

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
    TArray<AActor *> AttahcedActors;
    Pawn->GetAttachedActors(AttahcedActors);
    for (auto &Actor : AttahcedActors)
    {
        if (auto Container = Cast<AVFPhotoContainer>(Actor))
        {
            Execute_EndAiming(this, Controller);
            Container->AddAPhoto(this);
            Container->SetEnabled(true);
            return true;
        }
    }
    return false;
}