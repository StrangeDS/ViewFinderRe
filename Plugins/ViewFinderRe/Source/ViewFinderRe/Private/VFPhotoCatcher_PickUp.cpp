#include "VFPhotoCatcher_PickUp.h"

#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

#include "VFPhotoContainer.h"
#include "VFPhotoCaptureComponent.h"

bool AVFPhotoCatcher_PickUp::Interact_Implementation(APlayerController *Controller)
{
    EnableInteract(false);
    Execute_EndAiming(this, Controller);
    PlayerController = Controller;
    Pawn = PlayerController->GetPawn();
    ActorsToIgnore.AddUnique(Pawn);
    // 需要根据角色重写
    if (auto ToAttach = Pawn->GetComponentByClass<UCameraComponent>())
    {
        PickUp(ToAttach);
        // ToDo: 查找角色身上的Container.
        TArray<AActor*> AttahcedActors;
        Pawn->GetAttachedActors(AttahcedActors);
        for (const auto &Actor : AttahcedActors)
        {
            if (Actor->GetClass()->IsChildOf(AVFPhotoContainer::StaticClass()))
            {
                Container = Cast<AVFPhotoContainer>(Actor);
                return true;
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("AVFPhotoCatcher_PickUp::Interact_Implementation() has something wrong."));
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
        Container->SetEnabled(true);
        LeaveFromPreview();
    }

    return nullptr;
}

void AVFPhotoCatcher_PickUp::CloseToPreview_Implementation()
{
    Super::CloseToPreview_Implementation();

    GetWorldTimerManager().SetTimer(
        PreviewTimeHandle, [this]()
        {
            bReady = true;
            SetViewFrustumVisible(true);
            PhotoCapture->StartDraw();
        },
        TimeOfClose,
        false);
}

void AVFPhotoCatcher_PickUp::LeaveFromPreview_Implementation()
{
    Super::LeaveFromPreview_Implementation();

    GetWorldTimerManager().ClearTimer(PreviewTimeHandle);
    bReady = false;
    PhotoCapture->EndDraw();
    SetViewFrustumVisible(false);
}

void AVFPhotoCatcher_PickUp::PickUp_Implementation(USceneComponent *ToAttach)
{
    if (bPickedUp)
        return;

    AttachToComponent(ToAttach, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    RootComponent->SetRelativeLocation(FVector(10.0f, 20.0f, 0.0f)); // 用于测试

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
    ActorsToIgnore.AddUnique(GetAttachParentActor());
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    Pawn = nullptr;
    PlayerController = nullptr;
    bPickedUp = false;
    EnableInteract(true);
}
