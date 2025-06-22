#include "VFPhotoCatcher.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"

#include "VFCommon.h"
#include "VFDynamicMeshComponent.h"
#include "VFPhoto2D.h"
#include "VFPhoto3D.h"
#include "VFPhotoCaptureComponent.h"
#include "VFViewFrustumComponent.h"
#include "VFFunctions.h"
#include "VFHelperComponent.h"
#include "VFStandInInterface.h"
#include "VFPawnStandIn.h"
#include "VFHelperComponent.h"

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
		TEXT("/ViewFinderRe/StaticMeshes/CameraT.CameraT"));
	CatcherMesh = MeshSelector.Object;
	StaticMesh->SetStaticMesh(CatcherMesh);

	PhotoCapture = CreateDefaultSubobject<UVFPhotoCaptureComponent>(TEXT("PhotoCapture"));
	PhotoCapture->SetupAttachment(RootComponent);

	ViewFrustum = CreateDefaultSubobject<UVFViewFrustumComponent>(TEXT("ViewFrustum"));
	ViewFrustum->SetupAttachment(RootComponent);
	ViewFrustum->SetHiddenInGame(true);

	Helper = CreateDefaultSubobject<UVFHelperComponent>("Helper");

	VFDMCompClass = UVFDynamicMeshComponent::StaticClass();
	VFPhoto2DClass = AVFPhoto2D::StaticClass();
	VFPhoto3DClass = AVFPhoto3D::StaticClass();
}

void AVFPhotoCatcher::OnConstruction(const FTransform &Transform)
{
	Super::OnConstruction(Transform);

	ResetActorsToIgnore();
	ViewFrustum->RegenerateViewFrustum(ViewAngle, AspectRatio, StartDis, EndDis);
	PhotoCapture->FOVAngle = ViewAngle;
	PhotoCapture->CustomNearClippingPlane = StartDis;
	PhotoCapture->MaxViewDistanceOverride = EndDis;
	PhotoCapture->TargetHeight = PhotoCapture->TargetWidth / AspectRatio;
}

void AVFPhotoCatcher::BeginPlay()
{
	Super::BeginPlay();

	check(VFDMCompClass.Get());
	check(VFPhoto2DClass.Get());
	check(VFPhoto3DClass.Get());

	SetViewFrustumVisible(false);
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

AVFPhoto2D *AVFPhotoCatcher::TakeAPhoto_Implementation()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AVFPhotoCatcher::TakeAPhoto_Implementation()"));

	// 重叠检测
	TArray<UPrimitiveComponent *> OverlapComps = GetOverlapComps();

	// Helper筛选
	TMap<UPrimitiveComponent *, UVFHelperComponent *> HelperMap;
	UVFFunctions::GetCompsToHelpersMapping<UPrimitiveComponent>(OverlapComps, HelperMap);

	// 处理Helper相关设置
	{
		TArray<AActor *> ActorsNotTakenInPhoto = ActorsToIgnore;
		TMap<AActor *, UPrimitiveComponent *> StandInCompsMap;
		for (auto It = OverlapComps.CreateIterator(); It; It++)
		{
			auto Comp = *It;
			auto *HelperComp = HelperMap.Find(Comp); // 可能为nullptr

			// 剔除不显示的Actor
			if (HelperComp && !HelperMap[Comp]->bCanShowInPhoto)
			{
				ActorsNotTakenInPhoto.AddUnique(Comp->GetOwner());
			}

			// 剔除不进入后续的Actor
			if (bOnlyOverlapWithHelps && !HelperComp)
			{
				It.RemoveCurrent();
			}
			else if (HelperComp && !HelperMap[Comp]->bCanBeTakenInPhoto)
			{
				It.RemoveCurrent();
			}
			else if (HelperComp && HelperMap[Comp]->bReplacedWithStandIn)
			{
				// StandIn处理: 剔除自己的组件, 使用替身的组件.
				It.RemoveCurrent();
				auto Actor = Comp->GetOwner();
				if (!StandInCompsMap.Contains(Actor))
				{
					if (HelperMap[Comp]->bIgnoreChildActors)
					{
						TArray<AActor *> ChildActors;
						Actor->GetAttachedActors(ChildActors, true, true); // 递归是否应该支持配置?
						ActorsNotTakenInPhoto.Append(ChildActors);
					}

					auto StandIn = UVFFunctions::ReplaceWithStandIn(Actor, HelperMap[Comp]->StandInClass);
					auto StandInComp = IVFStandInInterface::Execute_GetPrimitiveComp(StandIn);
					StandInCompsMap.Add(Actor, StandInComp);
				}
			}
		}

		for (auto &[Actor, Comp] : StandInCompsMap)
		{
			OverlapComps.AddUnique(Comp);
		}
		PhotoCapture->HiddenActors = ActorsNotTakenInPhoto;
	}

	// (替身相关)需要重新生成.
	TSet<UVFHelperComponent *> HelpersRecorder;
	{
		HelperMap.Reset();
		UVFFunctions::GetCompsToHelpersMapping<UPrimitiveComponent>(OverlapComps, HelperMap);
		GetMapHelpers(HelperMap, HelpersRecorder);
	}

	// 拍照
	AVFPhoto2D *Photo2D = GetWorld()->SpawnActor<AVFPhoto2D>(
		VFPhoto2DClass.Get(),
		ViewFrustum->GetComponentLocation(),
		ViewFrustum->GetComponentRotation());
	{
		for (auto &HelperComp : HelpersRecorder)
		{
			HelperComp->NotifyDelegate(this, FVFHelperDelegateType::OriginalBeforeTakingPhoto);
		}
		Photo2D->SetPhoto(PhotoCapture);
	}

	// 基元组件下创建对应VFDynamicMeshComponent
	TArray<UVFDynamicMeshComponent *> VFDMComps;
	{
		for (auto &HelperComp : HelpersRecorder)
		{
			HelperComp->NotifyDelegate(this, FVFHelperDelegateType::OriginalBeforeCheckVFDMComps);
		}
		VFDMComps = UVFFunctions::CheckVFDMComps(OverlapComps, VFDMCompClass);
	}

	// 需要排序吗?
	// Algo::Sort(VFDMComps, [](UVFDynamicMeshComponent *A, UVFDynamicMeshComponent *B)
	// 			{ return A->GetOwner()->GetName() < B->GetOwner()->GetName(); });
	// for (auto Comp: VFDMComps)
	// {
	// 	VF_LOG(Warning, TEXT("%s"), *Comp->GetOwner()->GetName());
	// }

	// 复制对应Actor
	TArray<UVFDynamicMeshComponent *> CopiedComps;
	AVFPhoto3D *Photo3D = GetWorld()->SpawnActor<AVFPhoto3D>(
		VFPhoto3DClass.Get(),
		ViewFrustum->GetComponentLocation(),
		ViewFrustum->GetComponentRotation());
	{
		for (auto &HelperComp : HelpersRecorder)
		{
			HelperComp->NotifyDelegate(this, FVFHelperDelegateType::OriginalBeforeBeingCopied);
		}
		auto ActorsCopied = UVFFunctions::CopyActorsFromVFDMComps(GetWorld(), VFDMComps, CopiedComps);
		for (auto &Actor : ActorsCopied)
		{
			if (!ActorsCopied.Contains(Actor->GetAttachParentActor()))
				Actor->AttachToActor(Photo3D, FAttachmentTransformRules::KeepWorldTransform);
		}
	}

	// 原VFDynamicMeshComponent做切割
	if (bCuttingOrignal)
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

	// 新VFDynamicMeshComponent做交集
	TMap<UPrimitiveComponent *, UVFHelperComponent *> CopiedHelperMap;
	UVFFunctions::GetCompsToHelpersMapping<UVFDynamicMeshComponent>(CopiedComps, CopiedHelperMap);
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

	// Photo2D和Photo3D的后续处理
	{
		for (auto &HelperComp : CopiedHelpersRecorder)
		{
			HelperComp->NotifyDelegate(Photo3D, FVFHelperDelegateType::CopyBeforeFoldedInPhoto);
		}
		Photo2D->SetPhoto3D(Photo3D);
		Photo3D->RecordProperty(ViewFrustum, bOnlyOverlapWithHelps, ObjectTypesToOverlap);
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

void AVFPhotoCatcher::ResetActorsToIgnore()
{
	ActorsToIgnore.Reset();
	ActorsToIgnore.AddUnique(this);
}

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

UMaterialInstanceDynamic *AVFPhotoCatcher::GetScreenMID_Implementation()
{
	if (!ScreenMID)
		ScreenMID = StaticMesh->CreateDynamicMaterialInstance(1);
	return ScreenMID;
}

UVFHelperComponent *AVFPhotoCatcher::GetHelper_Implementation()
{
	return Helper;
}