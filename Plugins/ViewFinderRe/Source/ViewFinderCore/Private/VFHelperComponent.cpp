#include "VFHelperComponent.h"

UVFHelperComponent::UVFHelperComponent(const FObjectInitializer& ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UVFHelperComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UVFHelperComponent::BeginDestroy()
{
	OnOriginalBeforeTakenInPhoto.Clear();
	OnOriginalBeforeCopyingToPhoto.Clear();
	OnOriginalAfterCutByPhoto.Clear();
	OnOriginalAfterTakingPhoto.Clear();
	OnCopyAfterCopiedForPhoto.Clear();
	OnCopyBeforeFoldedInPhoto.Clear();
	OnCopyAfterPlacedByPhoto.Clear();

	Super::BeginDestroy();
}

void UVFHelperComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UVFHelperComponent::NotifyDelegate(const FVFHelperDelegateType &Type)
{
	bool IsHandled = false;
	switch (Type)
	{
	case FVFHelperDelegateType::OriginalBeforeTakenInPhoto:
		OnOriginalBeforeTakenInPhoto.Broadcast();
		IsHandled = true;
		break;
	case FVFHelperDelegateType::OriginalBeforeCopyingToPhoto:
		OnOriginalBeforeCopyingToPhoto.Broadcast();
		IsHandled = true;
		break;
	case FVFHelperDelegateType::OriginalAfterCutByPhoto:
		OnOriginalAfterCutByPhoto.Broadcast();
		IsHandled = true;
		break;
	case FVFHelperDelegateType::OriginalAfterTakingPhoto:
		OnOriginalAfterTakingPhoto.Broadcast();
		IsHandled = true;
		break;
	case FVFHelperDelegateType::CopyAfterCopiedForPhoto:
		OnCopyAfterCopiedForPhoto.Broadcast();
		IsHandled = true;
		break;
	case FVFHelperDelegateType::CopyBeforeFoldedInPhoto:
		OnCopyBeforeFoldedInPhoto.Broadcast();
		IsHandled = true;
		break;
	case FVFHelperDelegateType::CopyAfterPlacedByPhoto:
		OnCopyAfterPlacedByPhoto.Broadcast();
		IsHandled = true;
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("UVFHelperComponent::NotifyDelegate() don't handle."));
		break;
	}
	return IsHandled;
}