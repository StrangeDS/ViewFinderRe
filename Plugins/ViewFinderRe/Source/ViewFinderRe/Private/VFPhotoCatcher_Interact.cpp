#include "VFPhotoCatcher_Interact.h"

#include "Blueprint/UserWidget.h"
#include "EnhancedInputSubsystems.h"

#include "VFCommon.h"

bool AVFPhotoCatcher_Interact::StartAiming_Implementation(APlayerController *Controller)
{
	if (!AimingHintUMGClass.Get())
		return false;

	if (!AimingHintUMG)
		AimingHintUMG = CreateWidget<UUserWidget>(GetWorld(), AimingHintUMGClass);
	AimingHintUMG->AddToViewport();

	if (AimingMappingContext)
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
	if (AimingHintUMG)
		AimingHintUMG->RemoveFromParent();

	if (AimingMappingContext)
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
	if (!Pawn || !PlayerController)
	{
		VF_LOG(Warning, TEXT("%s: No Pawn or No PlayerController"), __FUNCTIONW__);
		return;
	}

	Pawn->DisableInput(PlayerController);
	// PlayerController->SetViewTargetWithBlend(this, TimeOfClose);
}

void AVFPhotoCatcher_Interact::LeaveFromPreview_Implementation()
{
	if (!Pawn || !PlayerController)
	{
		VF_LOG(Warning, TEXT("%s: No Pawn or No PlayerController"), __FUNCTIONW__);
		return;
	}

	// PlayerController->SetViewTargetWithBlend(Pawn, TimeOfLeave);
	Pawn->EnableInput(PlayerController);
}