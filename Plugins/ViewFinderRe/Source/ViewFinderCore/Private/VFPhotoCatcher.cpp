#include "VFPhotoCatcher.h"

#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"

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
#include "VFBackgroundCaptureComponent.h"
#include "VFPostProcessComponent.h"

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

	ActorsToIgnore.AddUnique(this);
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

void AVFPhotoCatcher::BeginPlay()
{
	Super::BeginPlay();

	check(VFDMCompClass.Get());
	check(VFPhoto2DClass.Get());
	check(VFPhoto3DClass.Get());

	PhotoCapture->HiddenActors = ActorsToIgnore;
	SetViewFrustumVisible(false);

	Helper->OnOriginalBeforeCheckVFDMComps.AddUniqueDynamic(
		this,
		&AVFPhotoCatcher::HandleOriginalBeforeCheckVFDMComps);
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

	/*
	Helper筛选
	注意! 查看: UVFFunctions.h Line 89的宏
	*/
	TMap<UPrimitiveComponent *, UVFHelperComponent *> HelperMap;
	UVFFunctions::GetCompsToHelpersMapping<UPrimitiveComponent>(OverlapComps, HelperMap);

	// 处理Helper相关设置
	{
		TArray<AActor *> ActorsToHide;
		TMap<AActor *, UPrimitiveComponent *> StandInCompsMap;
		for (auto It = OverlapComps.CreateIterator(); It; It++)
		{
			auto Comp = *It;
			bool HasHelper = HelperMap.Contains(Comp);

			// 剔除不显示的Actor
			bool HideOriginal = !HasHelper ||
								(HelperMap[Comp]->ShowInPhotoRule != FVFShowInPhotoRule::OriginalOnly &&
								 HelperMap[Comp]->ShowInPhotoRule != FVFShowInPhotoRule::Both);
			if (HideOriginal)
			{
				ActorsToHide.AddUnique(Comp->GetOwner());
			}

			// 剔除不进入后续的Actor
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
				// StandIn处理: 剔除自己的组件, 使用替身的组件.
				It.RemoveCurrent();
				auto Actor = Comp->GetOwner();
				if (!StandInCompsMap.Contains(Actor))
				{
					if (HelperMap[Comp]->bIgnoreChildActors)
					{
						TArray<AActor *> ChildActors;
						Actor->GetAttachedActors(ChildActors, true, true); // 递归是否应该支持配置?
						ActorsToHide.Append(ChildActors);
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
		PhotoCapture->HiddenActors.Append(ActorsToHide);
	}

	return OverlapComps;
}

AVFPhoto2D *AVFPhotoCatcher::TakeAPhoto_Implementation()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AVFPhotoCatcher::TakeAPhoto_Implementation()"));

	// 预生成Photo2D, 方便调试
	AVFPhoto2D *Photo2D = GetWorld()->SpawnActor<AVFPhoto2D>(
		VFPhoto2DClass.Get(),
		ViewFrustum->GetComponentLocation(),
		ViewFrustum->GetComponentRotation());

	// 重叠检测
	TArray<UPrimitiveComponent *> OverlapComps = GetOverlapComps();

	// 筛选组件, 拍照前准备
	OverlapComps = FilterOverlapComps(OverlapComps);

	// 生成HelperMap
	TMap<UPrimitiveComponent *, UVFHelperComponent *> HelperMap;
	TSet<UVFHelperComponent *> HelpersRecorder;
	{
		UVFFunctions::GetCompsToHelpersMapping<UPrimitiveComponent>(OverlapComps, HelperMap);
		GetMapHelpers(HelperMap, HelpersRecorder);
	}

	// 基元组件下创建对应VFDynamicMeshComponent
	TArray<UVFDynamicMeshComponent *> VFDMComps;
	{
		for (auto &HelperComp : HelpersRecorder)
		{
			HelperComp->NotifyDelegate(this, FVFHelperDelegateType::OriginalBeforeCheckVFDMComps);
		}
		VFDMComps = UVFFunctions::CheckVFDMComps(OverlapComps, VFDMCompClass);
		OverlapComps.Reset();
	}

	// 需要排序吗?
	// Algo::Sort(VFDMComps, [](UVFDynamicMeshComponent *A, UVFDynamicMeshComponent *B)
	// 			{ return A->GetOwner()->GetName() < B->GetOwner()->GetName(); });
	// for (auto Comp: VFDMComps)
	// {
	// 	VF_LOG(Warning, TEXT("%s"), *Comp->GetOwner()->GetName());
	// }

	// 重新生成HelperMap
	HelperMap.Reset();
	HelpersRecorder.Reset();
	{
		UVFFunctions::GetCompsToHelpersMapping<UVFDynamicMeshComponent>(VFDMComps, HelperMap);
		GetMapHelpers(HelperMap, HelpersRecorder);
	}

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

	// 拍照流程: 设置Stencil, 只对复制出来的Actor进行拍照
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
			// 注意: 对于生成的副本, 它应该使用原本的FVFShowInPhotoRule设置.
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
	Photo2D->SetPhoto(PhotoCapture);
	PhotoCapture->HiddenActors = ActorsToIgnore;

	// 背景绘制. 注意如果在拍摄照片前, 同一帧绘制会导致照片捕捉为未渲染状态.
	auto Plane = BackgroundCapture->DrawABackground();
	PostProcess->SetStencilValueNext(Plane);
	Plane->GetOwner()->AttachToActor(Photo3D, FAttachmentTransformRules::KeepWorldTransform);

	// Photo2D和Photo3D的后续处理
	{
		for (auto &HelperComp : CopiedHelpersRecorder)
		{
			HelperComp->NotifyDelegate(Photo3D, FVFHelperDelegateType::CopyBeforeFoldedInPhoto);
		}
		Photo2D->SetPhoto3D(Photo3D);
		Photo3D->RecordProperty(ViewFrustum, bOnlyOverlapWithHelper, ObjectTypesToOverlap);
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
	GetScreenMID();
}