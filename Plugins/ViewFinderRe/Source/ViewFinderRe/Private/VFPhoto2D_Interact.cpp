// Copyright 2026, StrangeDS. All Rights Reserved.

#include "VFPhoto2D_Interact.h"

#include "GameFramework/Pawn.h"
#include "Blueprint/UserWidget.h"

#include "VFPhotoContainerInterface.h"
#include "VFPhotoContainerManagerComponent.h"

void AVFPhoto2D_Interact::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (IsValid(AimingHintUMG))
    {
        AimingHintUMG->RemoveFromParent();
        AimingHintUMG->MarkAsGarbage();
    }

    Super::EndPlay(EndPlayReason);
}

bool AVFPhoto2D_Interact::StartAiming_Implementation(APlayerController *Controller)
{
    if (!AimingHintUMGClass.Get())
        return false;

    if (!IsValid(AimingHintUMG))
        AimingHintUMG = CreateWidget<UUserWidget>(GetWorld(), AimingHintUMGClass);
    AimingHintUMG->AddToViewport();

    return true;
}

bool AVFPhoto2D_Interact::EndAiming_Implementation(APlayerController *Controller)
{
    if (IsValid(AimingHintUMG))
        AimingHintUMG->RemoveFromParent();

    return true;
}

bool AVFPhoto2D_Interact::Interact_Implementation(APlayerController *Controller)
{
    return IsValid(Controller) && UVFPhotoContainerManagerComponent::TryTakeIn(Controller->GetPawn(), this);
}
