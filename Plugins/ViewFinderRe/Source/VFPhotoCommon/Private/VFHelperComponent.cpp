// Copyright 2026, StrangeDS. All Rights Reserved.

#include "VFHelperComponent.h"

#include "VFLog.h"
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
	OnOriginalBeforeCheckVFDMComps.Clear();
	OnOriginalBeforeBeingCopied.Clear();
	OnOriginalBeforeBegingCut.Clear();
	OnOriginalBeforeTakingPhoto.Clear();
	OnOriginalEndTakingPhoto.Clear();
	OnOriginalEndPlacingPhoto.Clear();
	OnCopyBeforeBeingCut.Clear();
	OnCopyBeforeTakingPhoto.Clear();
	OnCopyBeforeFoldedInPhoto.Clear();
	OnCopyEndTakingPhoto.Clear();
	OnCopyBeforeBeingEnabled.Clear();
	OnCopyEndPlacingPhoto.Clear();

	Super::EndPlay(EndPlayReason);
}

#if WITH_EDITOR
#include "Misc/DataValidation.h"

EDataValidationResult UVFHelperComponent::IsDataValid(FDataValidationContext &Context) const
{
	if (Super::IsDataValid(Context) == EDataValidationResult::Invalid)
		return EDataValidationResult::Invalid;

	if (bReplacedWithStandIn && !StandInClass.Get())
	{
		Context.AddError(FText::FromString(TEXT("StandInClass is invalid.")));
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}
#endif

bool UVFHelperComponent::NotifyDelegate(UObject *Sender, const FVFHelperDelegateType &Type)
{
	switch (Type)
	{
	case FVFHelperDelegateType::OriginalBeforeCheckVFDMComps:
		OnOriginalBeforeCheckVFDMComps.Broadcast(Sender);
		break;
	case FVFHelperDelegateType::OriginalBeforeBeingCopied:
		OnOriginalBeforeBeingCopied.Broadcast(Sender);
		break;
	case FVFHelperDelegateType::OriginalBeforeBegingCut:
		OnOriginalBeforeBegingCut.Broadcast(Sender);
		break;
	case FVFHelperDelegateType::OriginalBeforeTakingPhoto:
		OnOriginalBeforeTakingPhoto.Broadcast(Sender);
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
	case FVFHelperDelegateType::CopyBeforeTakingPhoto:
		OnCopyBeforeTakingPhoto.Broadcast(Sender);
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