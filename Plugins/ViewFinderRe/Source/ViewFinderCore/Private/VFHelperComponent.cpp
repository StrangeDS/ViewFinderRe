#include "VFHelperComponent.h"

#include "VFCommon.h"
#include "VFStandInInterface.h"

UVFHelperComponent::UVFHelperComponent(const FObjectInitializer &ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVFHelperComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bReplacedWithStandIn)
	{
		check(StandInClass.Get());
		check(StandInClass.Get()->ImplementsInterface(UVFStandInInterface::StaticClass()));
	}
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

bool UVFHelperComponent::NotifyDelegate(UObject *Sender, const FVFHelperDelegateType &Type)
{
	bool IsHandled = false;
	switch (Type)
	{
	case FVFHelperDelegateType::OriginalBeforeTakenInPhoto:
		OnOriginalBeforeTakenInPhoto.Broadcast(Sender);
		IsHandled = true;
		break;
	case FVFHelperDelegateType::OriginalBeforeCopyingToPhoto:
		OnOriginalBeforeCopyingToPhoto.Broadcast(Sender);
		IsHandled = true;
		break;
	case FVFHelperDelegateType::OriginalAfterCutByPhoto:
		OnOriginalAfterCutByPhoto.Broadcast(Sender);
		IsHandled = true;
		break;
	case FVFHelperDelegateType::OriginalAfterTakingPhoto:
		OnOriginalAfterTakingPhoto.Broadcast(Sender);
		IsHandled = true;
		break;
	case FVFHelperDelegateType::CopyAfterCopiedForPhoto:
		OnCopyAfterCopiedForPhoto.Broadcast(Sender);
		IsHandled = true;
		break;
	case FVFHelperDelegateType::CopyBeforeFoldedInPhoto:
		OnCopyBeforeFoldedInPhoto.Broadcast(Sender);
		IsHandled = true;
		break;
	case FVFHelperDelegateType::CopyAfterPlacedByPhoto:
		OnCopyAfterPlacedByPhoto.Broadcast(Sender);
		IsHandled = true;
		break;
	default:
		VF_LOG(Warning, TEXT("%s don't handle."), __FUNCTIONW__);
		break;
	}
	return IsHandled;
}