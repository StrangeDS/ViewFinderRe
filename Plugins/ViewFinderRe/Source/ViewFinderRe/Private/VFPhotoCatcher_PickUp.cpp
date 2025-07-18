#include "VFPhotoCatcher_PickUp.h"

#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "InputMappingContext.h"

#include "VFCommon.h"
#include "VFPhotoCaptureComponent.h"
#include "VFFunctions.h"
#include "VFCharacter.h"

bool AVFPhotoCatcher_PickUp::Interact_Implementation(APlayerController *Controller)
{
    EnableInteract(false);
    Execute_EndAiming(this, Controller);
    PlayerController = Controller;
    Pawn = PlayerController->GetPawn();
    ResetActorsToIgnore();
    ActorsToIgnore.AddUnique(Pawn);
    // 需要根据角色重写
    if (auto VFCharacter = Cast<AVFCharacter>(Pawn))
    {
        TScriptInterface<IVFActivatableInterface> Equipment(this);
        VFCharacter->AddEquipment(Equipment);
        VFCharacter->SwitchEquipment(Equipment);
        if (auto ToAttach = Pawn->GetComponentByClass<UCameraComponent>())
        {
            PickUp(ToAttach);
            return true;
        }
    }

    VF_LOG(Warning, TEXT("%s has something wrong."), __FUNCTIONW__);
    return false;
}

AVFPhoto2D *AVFPhotoCatcher_PickUp::TakeAPhoto_Implementation()
{
    if (!bReady)
        return nullptr;

    if (IsValid(Pawn) && Pawn->Implements<UVFPhotoContainerInterface>())
    {
        auto Photo2D = Super::TakeAPhoto_Implementation();
        IVFPhotoContainerInterface::Execute_TakeIn(Pawn, Photo2D);
    }
    LeaveFromPreview();

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
    EnableScreen(false);
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
    auto Transform = GetActorTransform();

    AttachToComponent(ToAttach, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    RootComponent->SetRelativeTransform(IdleTrans);
    bPickedUp = true;

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        FVFPhotoCatcherPickUpStepInfo Info(
            EVFPhotoCatcherPickUpOption::PickedUp,
            Transform,
            nullptr);
        StepInfos.Emplace(Info);
        StepsRecorder->SubmitStep(
            this,
            FVFStepInfo{
                EnumToString<EVFPhotoCatcherPickUpOption>(
                    EVFPhotoCatcherPickUpOption::PickedUp),
            });
    }
}

void AVFPhotoCatcher_PickUp::DropDown_Implementation()
{
    if (!bPickedUp)
        return;

    auto CompAttatched = RootComponent->GetAttachParent();

    ResetActorsToIgnore();
    ActorsToIgnore.AddUnique(GetAttachParentActor());
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    if (auto VFCharacter = Cast<AVFCharacter>(Pawn))
    {
        TScriptInterface<IVFActivatableInterface> Equipment(this);
        VFCharacter->RemoveEquipment(Equipment);
    }

    Pawn = nullptr;
    PlayerController = nullptr;
    bPickedUp = false;
    SetActorHiddenInGame(false);
    EnableInteract(true);

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        FVFPhotoCatcherPickUpStepInfo Info(
            EVFPhotoCatcherPickUpOption::DroppedDown,
            GetActorTransform(),
            CompAttatched);
        StepInfos.Emplace(Info);
        StepsRecorder->SubmitStep(
            this,
            FVFStepInfo{
                EnumToString<EVFPhotoCatcherPickUpOption>(
                    EVFPhotoCatcherPickUpOption::DroppedDown),
            });
    }
}

void AVFPhotoCatcher_PickUp::CloseToPreview_Move()
{
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

void AVFPhotoCatcher_PickUp::Activate_Implementation()
{
    SetActorHiddenInGame(false);

    if (IsValid(HoldingMappingContext))
    {
        auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
        if (Subsystem)
            Subsystem->AddMappingContext(HoldingMappingContext, 1);
        EnableInput(PlayerController);
    }

    AddPostProcessToPlayerCamera();
}

void AVFPhotoCatcher_PickUp::Deactivate_Implementation()
{
    RemovePostProcessFromPlayerCamera();

    if (IsValid(HoldingMappingContext))
    {
        DisableInput(PlayerController);
        auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
        if (Subsystem)
            Subsystem->RemoveMappingContext(HoldingMappingContext);
    }

    SetActorHiddenInGame(true);
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
        // AVFPhotoCatcherSteppable::TakeAPhoto_Implementation()中的提交, 无需处理
        return true;
    }

    auto Info = StepInfos.Pop(false);
    switch (Option)
    {
    case EVFPhotoCatcherPickUpOption::PickedUp:
    {
        DropDown();
        SetActorTransform(Info.Transform);
        return true;
    }
    case EVFPhotoCatcherPickUpOption::DroppedDown:
    {
        if (IsValid(Info.CompAttached))
        {
            PickUp(Info.CompAttached);
            return true;
        }
        return false;
    }
    default:
        return false;
    }
}