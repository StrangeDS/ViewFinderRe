// Copyright 2026, StrangeDS. All Rights Reserved.

#include "VFPhotoContainer_Input.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

AVFPhotoContainer_Input::AVFPhotoContainer_Input()
	: Super()
{
}

void AVFPhotoContainer_Input::SetEnabled(const bool &Enabled)
{
	Super::SetEnabled(Enabled);

	if (bEnabled)
	{
		auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (Subsystem)
			Subsystem->AddMappingContext(MappingContext, 2);
		EnableInput(PlayerController);
	}
	else
	{
		DisableInput(PlayerController);
		auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (Subsystem)
			Subsystem->RemoveMappingContext(MappingContext);
	}
}

void AVFPhotoContainer_Input::Activate_Implementation()
{
	SetEnabled(true);
}

void AVFPhotoContainer_Input::Deactivate_Implementation()
{
	SetEnabled(false);
}

bool AVFPhotoContainer_Input::CanActivate_Implementation()
{
	return Num() > 0;
}

bool AVFPhotoContainer_Input::IsActive_Implementation()
{
	return bEnabled;
}