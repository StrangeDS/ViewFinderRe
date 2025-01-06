#include "VFPhotoContainer_Input.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

AVFPhotoContainer_Input::AVFPhotoContainer_Input()
    :Super()
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
