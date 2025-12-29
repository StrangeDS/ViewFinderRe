// Copyright 2026, StrangeDS. All Rights Reserved.

#include "VFPhotoCatcher.h"

#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "VFLog.h"
#include "VFDynamicMeshComponent.h"
#include "VFPhoto2D.h"
#include "VFPhoto3D.h"
#include "VFPhotoCaptureComponent.h"
#include "VFViewFrustumComponent.h"
#include "VFPCatcherFunctions.h"
#include "VFHelperComponent.h"
#include "VFStandInInterface.h"
#include "VFPawnStandIn.h"
#include "VFHelperComponent.h"
#include "VFBackgroundCaptureComponent.h"
#include "VFPostProcessComponent.h"
#include "VFPhotoCatcherDeveloperSettings.h"

static void GetMapHelpers(
	const TMap<UPrimitiveComponent *, UVFHelperComponent *> &Map,
	TSet<UVFHelperComponent *> &Helpers)
{
	Helpers.Reset();
	for (auto &[Comp, Helper] : Map)
	{
		Helpers.Add(Helper);
	}
}

AVFPhotoCatcher::AVFPhotoCatcher()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(RootComponent);
	StaticMesh->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshSelector(
		TEXT("/ViewFinderRe/StaticMeshes/SM_Camera.SM_Camera"));
	CatcherMesh = MeshSelector.Object;
	StaticMesh->SetStaticMesh(CatcherMesh);

	PhotoCapture = CreateDefaultSubobject<UVFPhotoCaptureComponent>(TEXT("PhotoCapture"));
	PhotoCapture->SetupAttachment(RootComponent);

	BackgroundCapture = CreateDefaultSubobject<UVFBackgroundCaptureComponent>(TEXT("BackgroundCapture"));
	BackgroundCapture->SetupAttachment(PhotoCapture);

	ViewFrustum = CreateDefaultSubobject<UVFViewFrustumComponent>(TEXT("ViewFrustum"));
	ViewFrustum->SetupAttachment(RootComponent);
	ViewFrustum->SetHiddenInGame(true);

	Helper = CreateDefaultSubobject<UVFHelperComponent>("Helper");

	PostProcess = CreateDefaultSubobject<UVFPostProcessComponent>("PostProcess");

	VFDMCompClass = UVFDynamicMeshComponent::StaticClass();
	VFPhoto2DClass = AVFPhoto2D::StaticClass();
	VFPhoto3DClass = AVFPhoto3D::StaticClass();
}

void AVFPhotoCatcher::OnConstruction(const FTransform &Transform)
{
	Super::OnConstruction(Transform);

	ViewFrustum->RegenerateViewFrustum(ViewAngle, AspectRatio, StartDis, EndDis);
	PhotoCapture->FOVAngle = ViewAngle;
	PhotoCapture->CustomNearClippingPlane = StartDis;
	PhotoCapture->MaxViewDistanceOverride = EndDis;
	PhotoCapture->TargetHeight = PhotoCapture->TargetWidth / AspectRatio;

	BackgroundCapture->FOVAngle = ViewAngle;
	BackgroundCapture->AspectRatio = AspectRatio;
	BackgroundCapture->CustomNearClippingPlane = StartDis;
	BackgroundCapture->MaxViewDistanceOverride = EndDis;
	BackgroundCapture->TargetHeight = BackgroundCapture->TargetWidth / AspectRatio;

	PostProcess->ClearSceneCapturePostProcess();
	if (PostProcess->IsAnyRule())
	{
		PostProcess->AddOrUpdateSceneCapturePostProcess(PhotoCapture);
		PostProcess->AddOrUpdateSceneCapturePostProcess(BackgroundCapture);
	}
}

#if WITH_EDITOR
#include "Misc/DataValidation.h"

EDataValidationResult AVFPhotoCatcher::IsDataValid(FDataValidationContext &Context) const
{
	if (Super::IsDataValid(Context) == EDataValidationResult::Invalid)
		return EDataValidationResult::Invalid;

	bool AllValid = true;
	if (!VFDMCompClass.Get())
	{
		Context.AddError(FText::FromString(TEXT("VFDMCompClass is invalid.")));
		AllValid &= false;
	}
	if (!VFPhoto2DClass.Get())
	{
		Context.AddError(FText::FromString(TEXT("VFPhoto2DClass is invalid.")));
		AllValid &= false;
	}
	if (!VFPhoto3DClass.Get())
	{
		Context.AddError(FText::FromString(TEXT("VFPhoto3DClass is invalid.")));
		AllValid &= false;
	}

	return AllValid ? EDataValidationResult::Valid
					: EDataValidationResult::Invalid;
}
#endif

void AVFPhotoCatcher::BeginPlay()
{
	Super::BeginPlay();

	check(VFDMCompClass.Get());
	check(VFPhoto2DClass.Get());
	check(VFPhoto3DClass.Get());

	ActorsToIgnore.AddUnique(this);
	PhotoCapture->HiddenActors = ActorsToIgnore;
	SetViewFrustumVisible(false);

	Helper->OnOriginalBeforeCheckVFDMComps.AddUniqueDynamic(
		this,
		&AVFPhotoCatcher::HandleOriginalBeforeCheckVFDMComps);

	Helper->OnCopyEndPlacingPhoto.AddUniqueDynamic(
		this,
		&AVFPhotoCatcher::HandleCopyEndPlacingPhoto);
}

void AVFPhotoCatcher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

TArray<UPrimitiveComponent *> AVFPhotoCatcher::GetOverlapComps_Implementation()
{
	TArray<UPrimitiveComponent *> OverlapComps;
	UKismetSystemLibrary::ComponentOverlapComponents(
		ViewFrustum,
		ViewFrustum->GetComponentToWorld(),
		ObjectTypesToOverlap,
		UPrimitiveComponent::StaticClass(),
		ActorsToIgnore,
		OverlapComps);

	return OverlapComps;
}

TArray<UPrimitiveComponent *> AVFPhotoCatcher::FilterOverlapComps_Implementation(
	const TArray<UPrimitiveComponent *> &Comps)
{
	TArray<UPrimitiveComponent *> OverlapComps = Comps;

	TMap<UPrimitiveComponent *, UVFHelperComponent *> HelperMap;
	UVFPCatcherFunctions::GetCompsToHelpersMapping<UPrimitiveComponent>(OverlapComps, HelperMap);

	// Handle Helper configuration.
	{
		TArray<AActor *> ActorsToHide;
		TMap<AActor *, UPrimitiveComponent *> StandInCompsMap;
		for (auto It = OverlapComps.CreateIterator(); It; It++)
		{
			auto Comp = *It;
			bool HasHelper = HelperMap.Contains(Comp);

			// Cull Actors that do not participate in the photo-taking process and are not displayed;
			bool HideOriginal = !HasHelper ||
								(HelperMap[Comp]->ShowInPhotoRule != FVFShowInPhotoRule::OriginalOnly &&
								 HelperMap[Comp]->ShowInPhotoRule != FVFShowInPhotoRule::Both);
			if (HideOriginal)
			{
				ActorsToHide.AddUnique(Comp->GetOwner());
			}

			// Cull Actors that do not proceed to subsequent stages.
			if (bOnlyOverlapWithHelper && !HasHelper)
			{
				It.RemoveCurrent();
			}
			else if (HasHelper && !HelperMap[Comp]->bCanBeTakenInPhoto)
			{
				It.RemoveCurrent();
			}
			else if (HasHelper && HelperMap[Comp]->bReplacedWithStandIn)
			{
				// StandIn processing: Remove own component, use stand-in component.
				It.RemoveCurrent();
				auto Actor = Comp->GetOwner();
				if (!StandInCompsMap.Contains(Actor))
				{
					if (HelperMap[Comp]->bIgnoreChildActors)
					{
						TArray<AActor *> ChildActors;
						Actor->GetAttachedActors(ChildActors, true, true); // Should recursion be configurable?
						ActorsToHide.Append(ChildActors);
					}

					auto StandIn = UVFPCatcherFunctions::ReplaceWithStandIn(Actor, HelperMap[Comp]->StandInClass);
					auto StandInComp = IVFStandInInterface::Execute_GetPrimitiveComp(StandIn);
					StandInCompsMap.Add(Actor, StandInComp);
				}
			}
		}

		for (auto &[Actor, Comp] : StandInCompsMap)
		{
			OverlapComps.AddUnique(Comp);
		}
		PhotoCapture->HiddenActors.Append(ActorsToHide);
	}

	return OverlapComps;
}

AVFPhoto2D *AVFPhotoCatcher::TakeAPhoto_Implementation()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AVFPhotoCatcher::TakeAPhoto_Implementation()"));

	// Pre-generate Photo3D
	AVFPhoto3D *Photo3D = GetWorld()->SpawnActor<AVFPhoto3D>(
		VFPhoto3DClass.Get(),
		ViewFrustum->GetComponentLocation(),
		ViewFrustum->GetComponentRotation());
	Photo3D->RecordProperty(ViewFrustum, bOnlyOverlapWithHelper, ObjectTypesToOverlap);

	// Pre-generate Photo2D
	AVFPhoto2D *Photo2D = GetWorld()->SpawnActor<AVFPhoto2D>(
		VFPhoto2DClass.Get(),
		ViewFrustum->GetComponentLocation(),
		ViewFrustum->GetComponentRotation());
	Photo2D->SetPhoto3D(Photo3D);

	// Overlap detection
	TArray<UPrimitiveComponent *> OverlapComps = GetOverlapComps();

	// Filter components, preparation before taking photo
	OverlapComps = FilterOverlapComps(OverlapComps);

	// Generate HelperMap
	TMap<UPrimitiveComponent *, UVFHelperComponent *> HelperMap;
	TSet<UVFHelperComponent *> HelpersRecorder;
	{
		UVFPCatcherFunctions::GetCompsToHelpersMapping<UPrimitiveComponent>(OverlapComps, HelperMap);
		GetMapHelpers(HelperMap, HelpersRecorder);
	}

	// Create corresponding VFDynamicMeshComponent under primitive components
	TArray<UVFDynamicMeshComponent *> VFDMComps;
	{
		for (auto &HelperComp : HelpersRecorder)
		{
			HelperComp->NotifyDelegate(this, FVFHelperDelegateType::OriginalBeforeCheckVFDMComps);
		}
		VFDMComps = UVFPCatcherFunctions::CheckVFDMComps(OverlapComps, VFDMCompClass);
		OverlapComps.Reset();
	}

	// Regenerate HelperMap
	HelperMap.Reset();
	HelpersRecorder.Reset();
	{
		UVFPCatcherFunctions::GetCompsToHelpersMapping<UVFDynamicMeshComponent>(VFDMComps, HelperMap);
		GetMapHelpers(HelperMap, HelpersRecorder);
	}

	// Duplicate corresponding Actors
	TArray<UVFDynamicMeshComponent *> CopiedComps;
	{
		for (auto &HelperComp : HelpersRecorder)
		{
			HelperComp->NotifyDelegate(this, FVFHelperDelegateType::OriginalBeforeBeingCopied);
		}
		auto ActorsCopied = UVFPCatcherFunctions::CopyActorsFromVFDMComps(GetWorld(), VFDMComps, CopiedComps);
		for (auto &Actor : ActorsCopied)
		{
			if (!ActorsCopied.Contains(Actor->GetAttachParentActor()))
				Actor->AttachToActor(Photo3D, FAttachmentTransformRules::KeepWorldTransform);
		}
	}

	// Original VFDynamicMeshComponent do subtract operation.
	if (bCuttingOriginal)
	{
		for (auto &HelperComp : HelpersRecorder)
		{
			HelperComp->NotifyDelegate(this, FVFHelperDelegateType::OriginalBeforeBegingCut);
		}
		for (auto &Comp : VFDMComps)
		{
			Comp->SubtractMeshWithDMComp(ViewFrustum);
		}
	}

	// Copied VFDynamicMeshComponent do intersection operation.
	TMap<UPrimitiveComponent *, UVFHelperComponent *> CopiedHelperMap;
	UVFPCatcherFunctions::GetCompsToHelpersMapping<UVFDynamicMeshComponent>(CopiedComps, CopiedHelperMap);
	TSet<UVFHelperComponent *> CopiedHelpersRecorder;
	GetMapHelpers(CopiedHelperMap, CopiedHelpersRecorder);
	{
		for (auto &HelperComp : CopiedHelpersRecorder)
		{
			HelperComp->NotifyDelegate(this, FVFHelperDelegateType::CopyBeforeBeingCut);
		}

		for (auto &Comp : CopiedComps)
		{
			Comp->IntersectMeshWithDMComp(ViewFrustum);
		}
	}

	// capture for image: Set Stencil, and capture only the duplicated Actors.
	for (auto &HelperComp : HelpersRecorder)
	{
		HelperComp->NotifyDelegate(this, FVFHelperDelegateType::OriginalBeforeTakingPhoto);
	}
	for (auto &HelperComp : CopiedHelpersRecorder)
	{
		HelperComp->NotifyDelegate(this, FVFHelperDelegateType::CopyBeforeTakingPhoto);
	}
	if (PostProcess->IsAnyRule())
	{
		for (auto &Comp : CopiedComps)
		{
			PostProcess->SetStencilValueNext(Comp);
		}
	}
	{
		TArray<AActor *> ActorsToHide;
		for (int i = 0; i < VFDMComps.Num(); ++i)
		{
			// For generated duplicates, they should have the same FVFShowInPhotoRule as the originals.
			auto Comp = VFDMComps[i];
			auto CopiedComp = CopiedComps[i];
			check(Comp == CopiedComp->GetSourceComponent());
			bool HasHelper = HelperMap.Contains(Comp);
			bool HideOriginal = !HasHelper ||
								(HelperMap[Comp]->ShowInPhotoRule != FVFShowInPhotoRule::OriginalOnly &&
								 HelperMap[Comp]->ShowInPhotoRule != FVFShowInPhotoRule::Both);
			bool HideCopy = HasHelper &&
							HelperMap[Comp]->ShowInPhotoRule != FVFShowInPhotoRule::CopyOnly &&
							HelperMap[Comp]->ShowInPhotoRule != FVFShowInPhotoRule::Both;
			if (HideOriginal)
				ActorsToHide.AddUnique(Comp->GetOwner());
			if (HideCopy)
				ActorsToHide.AddUnique(CopiedComp->GetOwner());
		}
		ActorsToHide.AddUnique(Photo3D);
		PhotoCapture->HiddenActors.Append(ActorsToHide);
	}

	// Draw Photo2D image.
	Photo2D->SetPhoto(PhotoCapture);
	PhotoCapture->HiddenActors = ActorsToIgnore;

	// Draw planeActor image.
	bool GenerateAPlaneActor = bGenerateAPlaneActor &&
							   GetDefault<UVFPhotoCatcherDeveloperSettings>()->bGeneratePlaneActor;
	if (GenerateAPlaneActor)
	{
		auto Plane = BackgroundCapture->DrawABackground();
		if (PostProcess->IsAnyRule())
		{
			PostProcess->SetStencilValueNext(Plane);
		}
		Plane->GetOwner()->AttachToActor(Photo3D, FAttachmentTransformRules::KeepWorldTransform);
	}
	// Subsequent processing of Photo2D and Photo3D.
	{
		for (auto &HelperComp : CopiedHelpersRecorder)
		{
			HelperComp->NotifyDelegate(Photo3D, FVFHelperDelegateType::CopyBeforeFoldedInPhoto);
		}
		Photo2D->FoldUp();
	}

	{
		for (auto &HelperComp : HelpersRecorder)
		{
			HelperComp->NotifyDelegate(this, FVFHelperDelegateType::OriginalEndTakingPhoto);
		}
		for (auto &HelperComp : CopiedHelpersRecorder)
		{
			HelperComp->NotifyDelegate(this, FVFHelperDelegateType::CopyEndTakingPhoto);
		}
	}

	return Photo2D;
}

void AVFPhotoCatcher::SetViewFrustumVisible(const bool &Visibility)
{
	ViewFrustum->SetHiddenInGame(!Visibility);
}

#if WITH_EDITOR
void AVFPhotoCatcher::ResetActorsToIgnore()
{
	ActorsToIgnore.Reset();
	ActorsToIgnore.AddUnique(this);
}

void AVFPhotoCatcher::DrawABackgroundPlaneInEditor()
{
	UWorld *World = GetWorld();
	if (World->WorldType != EWorldType::Editor)
	{
		VF_LOG(Error, TEXT("%s is only called in editor."), __FUNCTIONW__);
		return;
	}

	FScopedTransaction Transaction(FText::FromString(TEXT("DrawABackgroundPlaneInEditor")));

	bool bUseTexture2DTemp = !IsValid(BackgroundCapture->TextureTarget);
	if (bUseTexture2DTemp)
		BackgroundCapture->Init();

	auto Plane = BackgroundCapture->DrawABackground();
	Plane->Modify();

	if (PostProcess->IsAnyRule())
	{
		PostProcess->SetStencilValueNext(Plane);
	}

	if (bUseTexture2DTemp)
		BackgroundCapture->TextureTarget = nullptr;
}
#endif

void AVFPhotoCatcher::EnableScreen(const bool &Enabled)
{
	if (!GetScreenMID())
	{
		VF_LOG(Error, TEXT("%s invalid ScreenMID."), __FUNCTIONW__);
	}

	if (Enabled)
	{
		ScreenMID->SetTextureParameterValue(TEXT("Texture"), PhotoCapture->TextureTarget);
		ScreenMID->SetScalarParameterValue(TEXT("AspectRatio"), AspectRatio);
		PhotoCapture->StartDraw();
	}
	else
	{
		PhotoCapture->EndDraw();
		ScreenMID->ClearParameterValues();
	}
}

FQuat AVFPhotoCatcher::GetFrustumQuat()
{
	return ViewFrustum->GetComponentQuat();
}

bool AVFPhotoCatcher::HasAnyLens()
{
	return PostProcess->IsAnyRule();
}

void AVFPhotoCatcher::PostProcessComps(TArray<UPrimitiveComponent *> Comps)
{
	for (auto Comp : Comps)
	{
		PostProcess->SetStencilValueNext(Comp);
	}
}

UMaterialInstanceDynamic *AVFPhotoCatcher::GetScreenMID_Implementation()
{
	if (!IsValid(ScreenMID))
		ScreenMID = StaticMesh->CreateDynamicMaterialInstance(1);
	return ScreenMID;
}

UVFHelperComponent *AVFPhotoCatcher::GetHelper_Implementation()
{
	return Helper;
}

void AVFPhotoCatcher::HandleOriginalBeforeCheckVFDMComps_Implementation(
	UObject *Sender)
{
	// Ensure photo2D's image synchronization after photo capture.
	GetScreenMID();
}

void AVFPhotoCatcher::HandleCopyEndPlacingPhoto_Implementation(UObject *Sender)
{
	// Regenerate ScreenMID, independent of the Original's dynamic material instance.
	ScreenMID = StaticMesh->CreateDynamicMaterialInstance(1, StaticMesh->GetMaterial(1)->GetMaterial());
	auto VFDynamicMeshComp = Cast<UVFDynamicMeshComponent>(StaticMesh->GetChildComponent(0));
	if (IsValid(VFDynamicMeshComp))
		VFDynamicMeshComp->SetMaterial(1, ScreenMID);
}