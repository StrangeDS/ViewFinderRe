// Copyright 2026, StrangeDS. All Rights Reserved.

#include "VFPhoto3D.h"

#include "Components/StaticMeshComponent.h"

#include "Kismet/KismetSystemLibrary.h"

#include "VFLog.h"
#include "VFDynamicMeshComponent.h"
#include "VFViewFrustumComponent.h"
#include "VFPCatcherFunctions.h"
#include "VFHelperComponent.h"

AVFPhoto3D::AVFPhoto3D() : Super()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent->SetMobility(EComponentMobility::Movable);
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(RootComponent);
	ViewFrustumRecorder = CreateDefaultSubobject<UVFViewFrustumComponent>(TEXT("ViewFrustum"));
	ViewFrustumRecorder->SetupAttachment(RootComponent);
	ViewFrustumRecorder->SetHiddenInGame(true);

	VFDMCompClass = UVFDynamicMeshComponent::StaticClass();
}

void AVFPhoto3D::BeginPlay()
{
	Super::BeginPlay();

	check(VFDMCompClass.Get());

	if (UVFPCatcherFunctions::IsEditorCreated(this))
	{
		SetVFDMCompsEnabled(false);
	}
}

#if WITH_EDITOR
#include "Misc/DataValidation.h"

EDataValidationResult AVFPhoto3D::IsDataValid(FDataValidationContext &Context) const
{
	if (Super::IsDataValid(Context) == EDataValidationResult::Invalid)
		return EDataValidationResult::Invalid;

	if (!VFDMCompClass.Get())
	{
		Context.AddError(FText::FromString(TEXT("VFDMCompClass is invalid.")));
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}
#endif

void AVFPhoto3D::FoldUp()
{
	if (State == EVFPhoto3DState::Folded)
		return;
	State = EVFPhoto3DState::Folded;

	SetVFDMCompsEnabled(false);
}

void AVFPhoto3D::PlaceDown()
{
	if (State == EVFPhoto3DState::Placed)
		return;
	State = EVFPhoto3DState::Placed;

	// Overlap Detection
	TArray<UPrimitiveComponent *> OverlapComps;
	UKismetSystemLibrary::ComponentOverlapComponents(
		ViewFrustumRecorder,
		ViewFrustumRecorder->GetComponentToWorld(),
		ObjectTypesToOverlap,
		UPrimitiveComponent::StaticClass(),
		ActorsToIgnore,
		OverlapComps);

	// Find Helpers of VFDynamicComps of external scene
	TArray<UVFHelperComponent *> HelpersPlaced;
	{
		TMap<UPrimitiveComponent *, UVFHelperComponent *> HelperMap;
		UVFPCatcherFunctions::GetCompsToHelpersMapping(OverlapComps, HelperMap);

		// Handle configuration related to Helpers
		for (auto It = OverlapComps.CreateIterator(); It; It++)
		{
			auto Comp = *It;
			auto Helper = HelperMap.Find(Comp); // May be nullptr
			if (bOnlyOverlapWithHelper && !Helper)
				It.RemoveCurrent();
			else if (Helper && !HelperMap[Comp]->bCanBePlacedByPhoto)
				It.RemoveCurrent();
		}

		HelpersPlaced.Reserve(HelperMap.Num());
		for (auto &[Comp, Helper] : HelperMap)
		{
			HelpersPlaced.Add(Helper);
		}
	}

	// Cutting VFDynamicComps of External Scene
	if (bCuttingOthers)
	{
		auto VFDMComps = UVFPCatcherFunctions::CheckVFDMComps(OverlapComps, VFDMCompClass);
		for (auto &Helper : HelpersPlaced)
		{
			Helper->NotifyDelegate(this, FVFHelperDelegateType::OriginalBeforeBegingCut);
		}
		for (auto Comp : VFDMComps)
		{
			Comp->SubtractMeshWithDMComp(ViewFrustumRecorder);
		}
	}

	// Find Actors and Helpers within Photo3D.
	TArray<UVFHelperComponent *> HelpersInPhoto3D;
	TArray<AActor *> ActorsInPhoto3D;
	{
		GetAttachedActors(ActorsInPhoto3D, true, true);
		for (const auto Actor : ActorsInPhoto3D)
		{
			if (auto Helper = Actor->GetComponentByClass<UVFHelperComponent>(); IsValid(Helper))
				HelpersInPhoto3D.AddUnique(Helper);
		}
	}

	// Enable Actors within Photo3D.
	{
		for (auto &Helper : HelpersInPhoto3D)
		{
			Helper->NotifyDelegate(this, FVFHelperDelegateType::CopyBeforeBeingEnabled);
		}
		SetVFDMCompsEnabled(true);
	}

	for (auto &Helper : HelpersPlaced)
	{
		Helper->NotifyDelegate(this, FVFHelperDelegateType::OriginalEndPlacingPhoto);
	}
	for (auto &Helper : HelpersInPhoto3D)
	{
		Helper->NotifyDelegate(this, FVFHelperDelegateType::CopyEndPlacingPhoto);
	}
}

void AVFPhoto3D::SetViewFrustumVisible(const bool &Visibility)
{
	ViewFrustumRecorder->SetHiddenInGame(!Visibility);
}

void AVFPhoto3D::SetVFDMCompsEnabled(const bool &Enabled, const bool IncludingActor)
{
	TArray<AActor *> Actors;
	GetAttachedActors(Actors, true, true);
	for (const auto &Actor : Actors)
	{
		TArray<UVFDynamicMeshComponent *> VFDMComps;
		Actor->GetComponents<UVFDynamicMeshComponent>(VFDMComps);
		for (const auto &Comp : VFDMComps)
		{
			Comp->SetEnabled(Enabled);
		}
		if (IncludingActor)
		{
			Actor->SetActorEnableCollision(Enabled);
			Actor->SetActorHiddenInGame(!Enabled);
		}
	}
}

void AVFPhoto3D::RecordProperty(
	UVFViewFrustumComponent *ViewFrustum,
	bool OnlyWithHelps,
	const TArray<TEnumAsByte<EObjectTypeQuery>> &ObjectTypes,
	EVFPhoto3DState StateIn)
{
	ViewFrustumRecorder->RecordViewFrustum(ViewFrustum);
	bOnlyOverlapWithHelper = OnlyWithHelps;
	ObjectTypesToOverlap = ObjectTypes;
	State = StateIn;
}