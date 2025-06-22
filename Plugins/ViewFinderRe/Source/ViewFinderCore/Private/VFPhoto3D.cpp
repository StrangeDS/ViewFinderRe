#include "VFPhoto3D.h"

#include "Components/StaticMeshComponent.h"

#include "Kismet/KismetSystemLibrary.h"

#include "VFCommon.h"
#include "VFDynamicMeshComponent.h"
#include "VFViewFrustumComponent.h"
#include "VFFunctions.h"
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
}

void AVFPhoto3D::FoldUp()
{
	if (State == EVFPhoto3DState::Folded)
		return;
	State = EVFPhoto3DState::Folded;

	SetVFDMCompsEnabled(false);
	TArray<AActor *> Actors;
	GetAttachedActors(Actors, true, true);
	for (const auto &Actor : Actors)
	{
		Actor->SetActorEnableCollision(false);
		Actor->SetActorHiddenInGame(true);
	}
}

void AVFPhoto3D::PlaceDown()
{
	if (State == EVFPhoto3DState::Placed)
		return;
	State = EVFPhoto3DState::Placed;

	// 重叠检测
	TArray<UPrimitiveComponent *> OverlapComps;
	UKismetSystemLibrary::ComponentOverlapComponents(
		ViewFrustumRecorder,
		ViewFrustumRecorder->GetComponentToWorld(),
		ObjectTypesToOverlap,
		UPrimitiveComponent::StaticClass(),
		ActorsToIgnore,
		OverlapComps);

	// 查找外部场景的Helpers
	TArray<UVFHelperComponent *> HelpersPlaced;
	{
		TMap<UPrimitiveComponent *, UVFHelperComponent *> HelperMap;
		UVFFunctions::GetCompsToHelpersMapping(OverlapComps, HelperMap);

		// 处理Helper相关设置
		for (auto It = OverlapComps.CreateIterator(); It; It++)
		{
			auto Comp = *It;
			auto Helper = HelperMap.Find(Comp); // 可能为nullptr
			if (bOnlyOverlapWithHelps && !Helper)
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

	// 外部场景切割
	{
		auto VFDMComps = UVFFunctions::CheckVFDMComps(OverlapComps, VFDMCompClass);
		for (auto &Helper : HelpersPlaced)
		{
			Helper->NotifyDelegate(this, FVFHelperDelegateType::OriginalBeforeBegingCut);
		}
		for (auto Comp : VFDMComps)
		{
			Comp->SubtractMeshWithDMComp(ViewFrustumRecorder);
		}
	}

	// 查找Photo3D内的Actors和Helpers
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

	// 启用Photo3D内的Actors
	{
		for (auto &Helper : HelpersInPhoto3D)
		{
			Helper->NotifyDelegate(this, FVFHelperDelegateType::CopyBeforeBeingEnabled);
		}

		for (auto &Actor : ActorsInPhoto3D)
		{
			Actor->SetActorEnableCollision(true);
			Actor->SetActorHiddenInGame(false);
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

void AVFPhoto3D::SetViewFrustumVisible(const bool &Visiblity)
{
	ViewFrustumRecorder->SetHiddenInGame(!Visiblity);
}

void AVFPhoto3D::SetVFDMCompsEnabled(const bool &Enabled)
{
	int Count = 0;
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
		Count = Count + VFDMComps.Num();
	}
}

void AVFPhoto3D::RecordProperty(
	UVFViewFrustumComponent *ViewFrustum,
	bool OnlyWithHelps,
	const TArray<TEnumAsByte<EObjectTypeQuery>> &ObjectTypes,
	EVFPhoto3DState StateIn)
{
	ViewFrustumRecorder->RecordViewFrustum(ViewFrustum);
	bOnlyOverlapWithHelps = OnlyWithHelps;
	ObjectTypesToOverlap = ObjectTypes;
	State = StateIn;
}
