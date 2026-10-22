// Copyright 2026, StrangeDS. All Rights Reserved.

#include "VFPhotoCatcher_PickUp.h"

#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Launch/Resources/Version.h"
#include "TimerManager.h"

#include "VFCharacter.h"
#include "VFEquipmentManagerComponent.h"
#include "VFLog.h"
#include "VFPCommonFunctions.h"
#include "VFPhotoCaptureComponent.h"
#include "VFPhotoContainerManagerComponent.h"

AVFPhotoCatcher_PickUp::AVFPhotoCatcher_PickUp()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    PrimaryActorTick.TickGroup = TG_PostUpdateWork;
}

bool AVFPhotoCatcher_PickUp::Interact_Implementation(APlayerController *Controller)
{
    if (bPickedUp || !IsValid(Controller) || !IsValid(Controller->GetPawn()))
    {
        return false;
    }
    APawn *NewPawn = Controller->GetPawn();
    UCameraComponent *Camera = NewPawn->FindComponentByClass<UCameraComponent>();
    if (!Camera && (!IsValid(Controller->PlayerCameraManager) || !NewPawn->GetRootComponent()))
    {
        return false;
    }
    EnableInteract(false);
    Execute_EndAiming(this, Controller);
    BindHolder(NewPawn, Controller);
    PickUp(Camera ? static_cast<USceneComponent *>(Camera) : Pawn->GetRootComponent());
    ConfigureViewTracking(Camera);
    RegisterWithEquipmentOwner();
    return true;
}

void AVFPhotoCatcher_PickUp::BindHolder(APawn *NewPawn, APlayerController *Controller)
{
    PlayerController = Controller;
    Pawn = NewPawn;
    Pawn->OnDestroyed.AddUniqueDynamic(this, &ThisClass::HandleHolderDestroyed);
    Pawn->ReceiveControllerChangedDelegate.AddUniqueDynamic(this, &ThisClass::HandleHolderControllerChanged);
    ActorsToIgnore.AddUnique(Pawn);
}

void AVFPhotoCatcher_PickUp::UnbindHolderCallbacks()
{
    // OnDestroyed can run while the pawn is pending destruction; its delegates still exist.
    if (Pawn)
    {
        Pawn->OnDestroyed.RemoveDynamic(this, &ThisClass::HandleHolderDestroyed);
        Pawn->ReceiveControllerChangedDelegate.RemoveDynamic(this, &ThisClass::HandleHolderControllerChanged);
    }
}

void AVFPhotoCatcher_PickUp::ConfigureViewTracking(UCameraComponent *Camera)
{
    bFollowCameraManager = !Camera;
    if (bFollowCameraManager)
    {
        DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        CameraRelativeTransform = IdleTrans;
        PrimaryActorTick.bCanEverTick = true;
        PrimaryActorTick.TickGroup = TG_PostUpdateWork;
        SetActorTickEnabled(true);
    }
}

void AVFPhotoCatcher_PickUp::RegisterWithEquipmentOwner()
{
    TScriptInterface<IVFActivatableInterface> Equipment(this);
    if (auto *Character = Cast<AVFCharacter>(Pawn))
    {
        Character->AddEquipment(Equipment);
        if (!bRestoringInactivePickup)
        {
            Character->SwitchEquipment(Equipment);
        }
    }
    else if (auto *Manager = Pawn->FindComponentByClass<UVFEquipmentManagerComponent>())
    {
        Manager->AddEquipment(Equipment);
        if (!bRestoringInactivePickup)
        {
            Manager->SwitchEquipment(Equipment);
        }
    }
    else if (!bRestoringInactivePickup)
    {
        IVFActivatableInterface::Execute_TryActivate(this);
    }
}

void AVFPhotoCatcher_PickUp::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bFollowCameraManager || !bPickedUp)
    {
        return;
    }
    if (!IsValid(Pawn) || !IsValid(PlayerController) || PlayerController->GetPawn() != Pawn)
    {
        ReleasePreviewInput();
        DropDown();
        return;
    }
    if (auto *Camera = PlayerController->PlayerCameraManager.Get())
    {
        const FTransform View(Camera->GetCameraRotation(), Camera->GetCameraLocation());
        SetActorTransform(CameraRelativeTransform * View);
    }
}

void AVFPhotoCatcher_PickUp::EndPlay(const EEndPlayReason::Type Reason)
{
    ReleasePreviewInput();
    if (IsValid(Pawn))
    {
        UnbindHolderCallbacks();
        if (auto *Manager = Pawn->FindComponentByClass<UVFEquipmentManagerComponent>())
        {
            Manager->RemoveEquipment(this);
        }
    }
    if (IsValid(PlayerController) && Execute_IsActive(this))
    {
        Execute_Deactivate(this);
    }
    GetWorldTimerManager().ClearTimer(PreviewTimeHandle);
    Super::EndPlay(Reason);
}

AVFPhoto2D *AVFPhotoCatcher_PickUp::TakeAPhoto_Implementation()
{
    if (!bReady || !IsValid(Pawn))
    {
        return nullptr;
    }
    auto *Photo = Super::TakeAPhoto_Implementation();
    UObject *Recipient = nullptr;
    if (UVFPhotoContainerManagerComponent::TryTakeIn(Pawn, Photo, &Recipient) && IsValid(Recipient) &&
        Recipient->Implements<UVFActivatableInterface>() &&
        IVFActivatableInterface::Execute_IsActive(Recipient))
    {
        Execute_Deactivate(this);
    }
    LeaveFromPreview();
    return Photo;
}

void AVFPhotoCatcher_PickUp::ReleasePreviewInput()
{
    if (bOwnsPreviewInput && IsValid(Pawn))
    {
        // UnPossess may have replaced the controller. nullptr is accepted by APawn::EnableInput.
        // bOwnsPreviewInput records whether this camera disabled the pawn's input boolean.
        Pawn->EnableInput(nullptr);
    }
    bOwnsPreviewInput = false;
}

void AVFPhotoCatcher_PickUp::HandleHolderDestroyed(AActor *Actor)
{
    ReleasePreviewInput();
    DropDown();
}

void AVFPhotoCatcher_PickUp::HandleHolderControllerChanged(APawn *Holder, AController *OldController,
                                                           AController *NewController)
{
    if (NewController != PlayerController)
    {
        ReleasePreviewInput();
        DropDown();
    }
}

void AVFPhotoCatcher_PickUp::CloseToPreview_Implementation()
{
    if (!IsValid(Pawn) || !IsValid(PlayerController))
    {
        return;
    }
    if (!bOwnsPreviewInput && Pawn->InputEnabled())
    {
        Pawn->DisableInput(PlayerController);
        bOwnsPreviewInput = true;
    }

    GetWorldTimerManager().ClearTimer(PreviewTimeHandle);
    GetWorldTimerManager().SetTimer(PreviewTimeHandle, this, &AVFPhotoCatcher_PickUp::CloseToPreview_Move,
                                    PreviewMoveInterval, true);
}

void AVFPhotoCatcher_PickUp::LeaveFromPreview_Implementation()
{
    ReleasePreviewInput();

    bReady = false;
    EnableScreen(false);
    SetViewFrustumVisible(false);

    GetWorldTimerManager().ClearTimer(PreviewTimeHandle);
    GetWorldTimerManager().SetTimer(PreviewTimeHandle, this, &AVFPhotoCatcher_PickUp::LeaveFromPreview_Move,
                                    PreviewMoveInterval, true);
}

void AVFPhotoCatcher_PickUp::PickUp_Implementation(USceneComponent *ToAttach)
{
    if (bPickedUp)
    {
        return;
    }
    auto Transform = GetActorTransform();
    auto Parent = GetAttachParentActor();

    AttachToComponent(ToAttach, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    RootComponent->SetRelativeTransform(IdleTrans);
    bPickedUp = true;

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        FVFPhotoCatcherPickUpStepInfo Info(EVFPhotoCatcherPickUpOption::PickedUp, Transform, Parent);
        StepInfos.Emplace(Info);
        StepsRecorder->SubmitStep(
            this, FVFStepInfo{
                      EnumToString<EVFPhotoCatcherPickUpOption>(EVFPhotoCatcherPickUpOption::PickedUp),
                  });
    }
}

void AVFPhotoCatcher_PickUp::DropDown_Implementation()
{
    if (!bPickedUp)
    {
        return;
    }

    // Ignore manual drops while pawn input is disabled. During rewind the default
    // GetStepsRecorder accessor returns nullptr, allowing a recorded pickup to be undone.
    if (IsValid(Pawn) && !Pawn->InputEnabled() && UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        return;
    }

    ReleasePreviewInput();
    UnbindHolderCallbacks();
    const bool bWasActive = Execute_IsActive(this);
    auto PC = PlayerController;

    ActorsToIgnore.AddUnique(GetAttachParentActor());
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    if (auto VFCharacter = Cast<AVFCharacter>(Pawn))
    {
        TScriptInterface<IVFActivatableInterface> Equipment(this);
        VFCharacter->RemoveEquipment(Equipment);
    }
    else if (IsValid(Pawn))
    {
        if (auto *Manager = Pawn->FindComponentByClass<UVFEquipmentManagerComponent>())
        {
            Manager->RemoveEquipment(this);
        }
    }
    if (IsValid(PlayerController) && Execute_IsActive(this))
    {
        Execute_Deactivate(this);
    }
    GetWorldTimerManager().ClearTimer(PreviewTimeHandle);
    bFollowCameraManager = false;
    SetActorTickEnabled(false);
    Pawn = nullptr;
    PlayerController = nullptr;
    bPickedUp = false;
    SetActorHiddenInGame(false);
    EnableInteract(true);

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        FVFPhotoCatcherPickUpStepInfo Info(EVFPhotoCatcherPickUpOption::DroppedDown, GetActorTransform(), PC,
                                           bWasActive);
        StepInfos.Emplace(Info);
        StepsRecorder->SubmitStep(
            this, FVFStepInfo{
                      EnumToString<EVFPhotoCatcherPickUpOption>(EVFPhotoCatcherPickUpOption::DroppedDown),
                  });
    }
}

void AVFPhotoCatcher_PickUp::CloseToPreview_Move()
{
    if (bFollowCameraManager)
    {
        const float Alpha =
            FMath::Clamp(PreviewMoveInterval / FMath::Max(TimeOfClose, SMALL_NUMBER), 0.f, 1.f);
        CameraRelativeTransform =
            UVFPCommonFunctions::TransformLerp(CameraRelativeTransform, PreviewTrans, Alpha);
        bReady = CameraRelativeTransform.Equals(PreviewTrans, 0.05f);
        if (bReady)
        {
            CameraRelativeTransform = PreviewTrans;
            GetWorldTimerManager().ClearTimer(PreviewTimeHandle);
            SetViewFrustumVisible(true);
            EnableScreen(true);
        }
        return;
    }
    auto TransCur = RootComponent->GetRelativeTransform();
    bReady = TransCur.Equals(PreviewTrans);
    if (bReady)
    {
        GetWorldTimerManager().ClearTimer(PreviewTimeHandle);

        SetViewFrustumVisible(true);
        EnableScreen(true);
    }
    else
    {
        auto Rate = GetWorldTimerManager().GetTimerRate(PreviewTimeHandle);
        Rate = 1 - Rate / TimeOfClose;
        auto TransTarget = UVFPCommonFunctions::TransformLerp(TransCur, PreviewTrans, Rate);
        RootComponent->SetRelativeTransform(TransTarget);
    }
}

void AVFPhotoCatcher_PickUp::LeaveFromPreview_Move()
{
    if (bFollowCameraManager)
    {
        const float Alpha =
            FMath::Clamp(PreviewMoveInterval / FMath::Max(TimeOfLeave, SMALL_NUMBER), 0.f, 1.f);
        CameraRelativeTransform =
            UVFPCommonFunctions::TransformLerp(CameraRelativeTransform, IdleTrans, Alpha);
        if (CameraRelativeTransform.Equals(IdleTrans, 0.05f))
        {
            CameraRelativeTransform = IdleTrans;
            GetWorldTimerManager().ClearTimer(PreviewTimeHandle);
        }
        return;
    }
    auto TransCur = RootComponent->GetRelativeTransform();
    if (TransCur.Equals(IdleTrans))
    {
        GetWorldTimerManager().ClearTimer(PreviewTimeHandle);
    }
    else
    {
        auto Rate = GetWorldTimerManager().GetTimerRate(PreviewTimeHandle);
        Rate = 1 - Rate / TimeOfLeave;
        auto TransTarget = UVFPCommonFunctions::TransformLerp(TransCur, IdleTrans, Rate);
        RootComponent->SetRelativeTransform(TransTarget);
    }
}

void AVFPhotoCatcher_PickUp::Activate_Implementation()
{
    if (!IsValid(PlayerController) || !IsValid(Pawn))
    {
        return;
    }
    SetActorHiddenInGame(false);

    if (IsValid(HoldingMappingContext))
    {
        auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
            PlayerController->GetLocalPlayer());
        if (Subsystem)
        {
            Subsystem->AddMappingContext(HoldingMappingContext, 1);
        }
        EnableInput(PlayerController);
    }

    AddPostProcessToPlayerCamera();

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        StepsRecorder->SubmitStep(
            this, FVFStepInfo{
                      EnumToString<EVFPhotoCatcherPickUpOption>(EVFPhotoCatcherPickUpOption::Activate),
                  });
    }
}

void AVFPhotoCatcher_PickUp::Deactivate_Implementation()
{
    if (!IsValid(PlayerController))
    {
        SetActorHiddenInGame(true);
        return;
    }
    RemovePostProcessFromPlayerCamera();

    if (IsValid(HoldingMappingContext))
    {
        DisableInput(PlayerController);
        auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
            PlayerController->GetLocalPlayer());
        if (Subsystem)
        {
            Subsystem->RemoveMappingContext(HoldingMappingContext);
        }
    }

    SetActorHiddenInGame(true);

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        StepsRecorder->SubmitStep(
            this, FVFStepInfo{
                      EnumToString<EVFPhotoCatcherPickUpOption>(EVFPhotoCatcherPickUpOption::Deactivate),
                  });
    }
}

bool AVFPhotoCatcher_PickUp::CanActivate_Implementation()
{
    return true;
}

bool AVFPhotoCatcher_PickUp::IsActive_Implementation()
{
    return !IsHidden();
}

bool AVFPhotoCatcher_PickUp::StepBack_Implementation(FVFStepInfo &StepInfo)
{
    auto Option = StringToEnum<EVFPhotoCatcherPickUpOption>(StepInfo.Info);
    if (Option >= EVFPhotoCatcherPickUpOption::MAX)
    {
        // No need to handle event by AVFPhotoCatcherSteppable::TakeAPhoto_Implementation().
        return true;
    }

    switch (Option)
    {
    case EVFPhotoCatcherPickUpOption::PickedUp:
    {
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 5
        auto Info = StepInfos.Pop(EAllowShrinking::No);
#else
        auto Info = StepInfos.Pop(false);
#endif
        DropDown();
        SetActorTransform(Info.Transform);
        if (IsValid(Info.ParentOrPlayerController))
        {
            AttachToActor(Info.ParentOrPlayerController, FAttachmentTransformRules::KeepWorldTransform);
        }
        return true;
    }
    case EVFPhotoCatcherPickUpOption::DroppedDown:
    {
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 5
        auto Info = StepInfos.Pop(EAllowShrinking::No);
#else
        auto Info = StepInfos.Pop(false);
#endif
        if (auto PC = Cast<APlayerController>(Info.ParentOrPlayerController); IsValid(PC))
        {
            // Restore the held camera without stealing the equipment selection for an inactive camera.
            TGuardValue<bool> RestoreGuard(bRestoringInactivePickup, !Info.bWasActive);
            Execute_Interact(this, PC);
            if (!Info.bWasActive)
            {
                SetActorHiddenInGame(true);
            }
            return true;
        }
        StepInfos.Pop();
        return false;
    }
    case EVFPhotoCatcherPickUpOption::Activate:
    {
        Execute_Deactivate(this);
        return true;
    }
    case EVFPhotoCatcherPickUpOption::Deactivate:
    {
        Execute_Activate(this);
        return true;
    }
    default:
        return false;
    }
}
