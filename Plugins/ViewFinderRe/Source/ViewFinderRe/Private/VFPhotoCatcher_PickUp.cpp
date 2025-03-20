#include "VFPhotoCatcher_PickUp.h"

#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

#include "VFCommon.h"
#include "VFPhotoContainer.h"
#include "VFPhotoCaptureComponent.h"
#include "VFFunctions.h"

bool AVFPhotoCatcher_PickUp::Interact_Implementation(APlayerController *Controller)
{
    EnableInteract(false);
    Execute_EndAiming(this, Controller);
    PlayerController = Controller;
    Pawn = PlayerController->GetPawn();
    ResetActorsToIgnore();
    ActorsToIgnore.AddUnique(Pawn);
    // 需要根据角色重写
    if (auto ToAttach = Pawn->GetComponentByClass<UCameraComponent>())
    {
        PickUp(ToAttach);
        TArray<AActor *> AttahcedActors;
        Pawn->GetAttachedActors(AttahcedActors);
        for (const auto &Actor : AttahcedActors)
        {
            if (Actor->GetClass()->IsChildOf(AVFPhotoContainer::StaticClass()))
            {
                Container = Cast<AVFPhotoContainer>(Actor);
                Container->SetEnabled(false);
                Container->OnEnabled.AddUniqueDynamic(this, &AVFPhotoCatcher_PickUp::SetActorHiddenInGame);
                return true;
            }
        }
    }

    VF_LOG(Warning, TEXT("%s has something wrong."), __FUNCTIONW__);
    return false;
}

AVFPhoto2D *AVFPhotoCatcher_PickUp::TakeAPhoto_Implementation()
{
    if (!bReady)
        return nullptr;

    if (ensure(Container))
    {
        auto Photo2D = Super::TakeAPhoto_Implementation();
        Container->AddAPhoto(Photo2D);
        Container->SetEnabled(Container->Num() == 1);
        LeaveFromPreview();
    }

    return nullptr;
}

void AVFPhotoCatcher_PickUp::CloseToPreview_Implementation()
{
	Pawn->DisableInput(PlayerController);

    GetWorldTimerManager().ClearTimer(PreviewTimeHandle);
    GetWorldTimerManager().SetTimer(
        PreviewTimeHandle,
        this,
        &AVFPhotoCatcher_PickUp::CloseToPreview_Move,
        PreviewMoveInterval,
        true);
}

void AVFPhotoCatcher_PickUp::LeaveFromPreview_Implementation()
{
	Pawn->EnableInput(PlayerController);

    bReady = false;
    PhotoCapture->EndDraw();
    SetViewFrustumVisible(false);

    GetWorldTimerManager().ClearTimer(PreviewTimeHandle);
    GetWorldTimerManager().SetTimer(
        PreviewTimeHandle,
        this,
        &AVFPhotoCatcher_PickUp::LeaveFromPreview_Move,
        PreviewMoveInterval,
        true);
}

void AVFPhotoCatcher_PickUp::PickUp_Implementation(USceneComponent *ToAttach)
{
    if (bPickedUp)
        return;

    AttachToComponent(ToAttach, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    RootComponent->SetRelativeTransform(IdleTrans);

    if (HoldingMappingContext)
    {
        auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
        if (Subsystem)
            Subsystem->AddMappingContext(HoldingMappingContext, 1);
        EnableInput(PlayerController);
    }
    bPickedUp = true;
}

void AVFPhotoCatcher_PickUp::DropDown_Implementation()
{
    if (!bPickedUp)
        return;

    if (HoldingMappingContext)
    {
        DisableInput(PlayerController);
        auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
        if (Subsystem)
            Subsystem->RemoveMappingContext(HoldingMappingContext);
    }
    ResetActorsToIgnore();
    ActorsToIgnore.AddUnique(GetAttachParentActor());
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    
    Container->OnEnabled.RemoveDynamic(this, &AVFPhotoCatcher_PickUp::SetActorHiddenInGame);
    Container = nullptr;

    Pawn = nullptr;
    PlayerController = nullptr;
    bPickedUp = false;
    EnableInteract(true);
}

void AVFPhotoCatcher_PickUp::CloseToPreview_Move()
{
    auto TransCur = RootComponent->GetRelativeTransform();
    bReady = TransCur.Equals(PreviewTrans);
    if (bReady)
    {
        GetWorldTimerManager().ClearTimer(PreviewTimeHandle);

        SetViewFrustumVisible(true);
        PhotoCapture->StartDraw();
    }
    else
    {
        auto Rate = GetWorldTimerManager().GetTimerRate(PreviewTimeHandle);
        Rate = 1 - Rate / TimeOfClose;
        auto TransTarget = UVFFunctions::TransformLerp(TransCur, PreviewTrans, Rate);
        RootComponent->SetRelativeTransform(TransTarget);
    }
}

void AVFPhotoCatcher_PickUp::LeaveFromPreview_Move()
{
    auto TransCur = RootComponent->GetRelativeTransform();
    if (TransCur.Equals(IdleTrans))
    {
        GetWorldTimerManager().ClearTimer(PreviewTimeHandle);
    }
    else
    {
        auto Rate = GetWorldTimerManager().GetTimerRate(PreviewTimeHandle);
        Rate = 1 - Rate / TimeOfLeave;
        auto TransTarget = UVFFunctions::TransformLerp(TransCur, IdleTrans, Rate);
        RootComponent->SetRelativeTransform(TransTarget);
    }
}
