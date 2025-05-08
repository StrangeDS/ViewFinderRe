#include "VFHelperComponent.h"

#include "VFCommon.h"
#include "VFPawnStandIn.h"
#include "VFStandInInterface.h"

UVFHelperComponent::UVFHelperComponent(const FObjectInitializer &ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;

	StandInClass = AVFPawnStandIn::StaticClass();
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

void UVFHelperComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnOriginalBeforeTakingPhoto.Clear();
	OnOriginalBeforeCheckVFDMComps.Clear();
	OnOriginalBeforeBeingCopied.Clear();
	OnOriginalBeforeBegingCut.Clear();
	OnOriginalEndTakingPhoto.Clear();
	OnOriginalEndPlacingPhoto.Clear();
	OnCopyBeforeBeingCut.Clear();
	OnCopyBeforeFoldedInPhoto.Clear();
	OnCopyEndTakingPhoto.Clear();
	OnCopyBeforeBeingEnabled.Clear();
	OnCopyEndPlacingPhoto.Clear();

	Super::EndPlay(EndPlayReason);
}

bool UVFHelperComponent::NotifyDelegate(UObject *Sender, const FVFHelperDelegateType &Type)
{
	switch (Type)
	{
	case FVFHelperDelegateType::OriginalBeforeTakingPhoto:
		OnOriginalBeforeTakingPhoto.Broadcast(Sender);
		break;
	case FVFHelperDelegateType::OriginalBeforeCheckVFDMComps:
		OnOriginalBeforeCheckVFDMComps.Broadcast(Sender);
		break;
	case FVFHelperDelegateType::OriginalBeforeBeingCopied:
		OnOriginalBeforeBeingCopied.Broadcast(Sender);
		break;
	case FVFHelperDelegateType::OriginalBeforeBegingCut:
		OnOriginalBeforeBegingCut.Broadcast(Sender);
		break;
	case FVFHelperDelegateType::OriginalEndTakingPhoto:
		OnOriginalEndTakingPhoto.Broadcast(Sender);
		break;
	case FVFHelperDelegateType::OriginalEndPlacingPhoto:
		OnOriginalEndPlacingPhoto.Broadcast(Sender);
		break;
	case FVFHelperDelegateType::CopyBeforeBeingCut:
		OnCopyBeforeBeingCut.Broadcast(Sender);
		break;
	case FVFHelperDelegateType::CopyBeforeFoldedInPhoto:
		OnCopyBeforeFoldedInPhoto.Broadcast(Sender);
		break;
	case FVFHelperDelegateType::CopyEndTakingPhoto:
		OnCopyEndTakingPhoto.Broadcast(Sender);
		break;
	case FVFHelperDelegateType::CopyBeforeBeingEnabled:
		OnCopyBeforeBeingEnabled.Broadcast(Sender);
		break;
	case FVFHelperDelegateType::CopyEndPlacingPhoto:
		OnCopyEndPlacingPhoto.Broadcast(Sender);
		break;
	case FVFHelperDelegateType::MAX:
	default:
		VF_LOG(Warning, TEXT("%s don't handle."), __FUNCTIONW__);
		return false;
	}
	return true;
}