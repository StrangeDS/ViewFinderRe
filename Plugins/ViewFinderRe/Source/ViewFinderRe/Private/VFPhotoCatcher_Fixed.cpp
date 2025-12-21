// Copyright StrangeDS. All Rights Reserved.

#include "VFPhotoCatcher_Fixed.h"

#include "TimerManager.h"

#include "VFPhoto2D.h"
#include "VFPhotoCaptureComponent.h"

AVFPhoto2D *AVFPhotoCatcher_Fixed::TakeAPhoto_Implementation()
{
    GetWorldTimerManager().ClearTimer(TimerHandleOfTakingPhoto);
    EnableScreen(true);
    GetWorldTimerManager().SetTimer(
        TimerHandleOfTakingPhoto, [this]()
        {
            EnableScreen(false);
            auto Photo = Super::TakeAPhoto_Implementation();
            Photo->AddActorLocalTransform(PhotoSpawnPoint); },
        TimeOfTakingPhoto,
        false);

    return nullptr;
}

bool AVFPhotoCatcher_Fixed::StartAiming_Implementation(APlayerController *Controller)
{
    Super::StartAiming_Implementation(Controller);

    SetViewFrustumVisible(true);
    return true;
}

bool AVFPhotoCatcher_Fixed::EndAiming_Implementation(APlayerController *Controller)
{
    Super::EndAiming_Implementation(Controller);

    SetViewFrustumVisible(false);
    return true;
}

bool AVFPhotoCatcher_Fixed::Interact_Implementation(APlayerController *Controller)
{
    TakeAPhoto();
    return true;
}