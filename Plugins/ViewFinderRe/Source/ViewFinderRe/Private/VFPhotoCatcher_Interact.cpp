#include "VFPhotoCatcher_Interact.h"

#include "Blueprint/UserWidget.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Camera/CameraComponent.h"

#include "VFLog.h"
#include "VFPostProcessComponent.h"

bool AVFPhotoCatcher_Interact::StartAiming_Implementation(APlayerController *Controller)
{
	if (!AimingHintUMGClass.Get())
		return false;

	if (!IsValid(AimingHintUMG))
		AimingHintUMG = CreateWidget<UUserWidget>(GetWorld(), AimingHintUMGClass);
	AimingHintUMG->AddToViewport();

	if (IsValid(AimingMappingContext))
	{
		// 具体事件的BindAction放在蓝图中进行(图个方便)
		auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Controller->GetLocalPlayer());
		if (Subsystem)
			Subsystem->AddMappingContext(AimingMappingContext, 1);
	}

	return true;
}

bool AVFPhotoCatcher_Interact::EndAiming_Implementation(APlayerController *Controller)
{
	if (IsValid(AimingHintUMG))
		AimingHintUMG->RemoveFromParent();

	if (IsValid(AimingMappingContext))
	{
		auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Controller->GetLocalPlayer());
		if (Subsystem)
			Subsystem->RemoveMappingContext(AimingMappingContext);
	}
	return true;
}

bool AVFPhotoCatcher_Interact::Interact_Implementation(APlayerController *Controller)
{
	return false;
}

bool AVFPhotoCatcher_Interact::IsEnabled_Implementation(APlayerController *Controller)
{
	return bInteractingEnabled;
}

void AVFPhotoCatcher_Interact::EnableInteract_Implementation(bool Enabled)
{
	if (bInteractingEnabled == Enabled)
		return;

	bInteractingEnabled = Enabled;
	SetActorEnableCollision(bInteractingEnabled);
}

void AVFPhotoCatcher_Interact::CloseToPreview_Implementation()
{
	check(Pawn && PlayerController);

	Pawn->DisableInput(PlayerController);
	PlayerController->SetViewTargetWithBlend(this, TimeOfClose);
}

void AVFPhotoCatcher_Interact::LeaveFromPreview_Implementation()
{
	check(Pawn && PlayerController);

	PlayerController->SetViewTargetWithBlend(Pawn, TimeOfLeave);
	Pawn->EnableInput(PlayerController);
}

void AVFPhotoCatcher_Interact::AddPostProcessToPlayerCamera()
{
	if (!ensureMsgf(PlayerController, TEXT("%s invalid PlayerController."), __FUNCTIONW__))
		return;

	if (PostProcess->IsAnyRule())
	{
		if (auto Camera = PlayerController->GetPawn()->GetComponentByClass<UCameraComponent>())
		{
			PostProcess->AddOrUpdateCameraPostProcess(Camera);
		}
	}
}

void AVFPhotoCatcher_Interact::RemovePostProcessFromPlayerCamera()
{
	if (!ensureMsgf(PlayerController, TEXT("%s invalid PlayerController."), __FUNCTIONW__))
		return;

	if (PostProcess->IsAnyRule())
	{
		if (auto Camera = PlayerController->GetPawn()->GetComponentByClass<UCameraComponent>())
		{
			PostProcess->RemoveCameraPostProcess(Camera);
		}
	}
}