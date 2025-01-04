#include "VFPhoto3D.h"

#include "Components/StaticMeshComponent.h"

#include "VFDynamicMeshComponent.h"
#include "VFViewFrustumComponent.h"

#include "Kismet/KismetSystemLibrary.h"
#include "VFFunctions.h"

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
}

void AVFPhoto3D::BeginPlay()
{
	check(VFDMCompClass.Get());

	Super::BeginPlay();
}

void AVFPhoto3D::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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

	TArray<UPrimitiveComponent *> OverlapComps;
	UKismetSystemLibrary::ComponentOverlapComponents(
		ViewFrustumRecorder,
		ViewFrustumRecorder->GetComponentToWorld(),
		ObjectTypesToOverlap,
		UPrimitiveComponent::StaticClass(),
		{},
		OverlapComps);

	TMap<UPrimitiveComponent *, UVFHelperComponent *> HelperMap;
	UVFFunctions::GetCompsToHelpersMapping(OverlapComps, HelperMap);

	for (auto It = OverlapComps.CreateIterator(); It; It++)
	{
		auto Comp = *It;
		auto Helper = HelperMap.Find(Comp); // 可能为nullptr
		if (bOnlyOverlapWithHelps && !Helper)
			It.RemoveCurrent();
		else if (Helper && !HelperMap[Comp]->bCanBePlacedByPhoto)
			It.RemoveCurrent();
	}

	TSet<UVFHelperComponent *> HelpersRecorder;
	for (auto &[Comp, Helper] : HelperMap)
	{
		HelpersRecorder.Add(Helper);
	}

	auto VFDMComps = UVFFunctions::CheckVFDMComps(OverlapComps, VFDMCompClass);
	UE_LOG(LogTemp, Warning, TEXT("PlaceDown overlaps %i"), VFDMComps.Num());

	for (auto Comp : VFDMComps)
	{
		Comp->SubtractMeshWithDMComp(ViewFrustumRecorder);
	}
	for (auto &Helper : HelpersRecorder)
	{
		Helper->NotifyDelegate(FVFHelperDelegateType::OriginalAfterCutByPhoto);
	}

	TArray<AActor *> Actors;
	GetAttachedActors(Actors, true, true);
	for (const auto &Actor : Actors)
	{
		Actor->SetActorEnableCollision(true);
		Actor->SetActorHiddenInGame(false);
	}
	SetVFDMCompsEnabled(true);

	for (const auto Actor : Actors)
	{
		if (auto Helper = Actor->GetComponentByClass<UVFHelperComponent>())
		{
			Helper->NotifyDelegate(FVFHelperDelegateType::CopyAfterPlacedByPhoto);
		}
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
	const TArray<TEnumAsByte<EObjectTypeQuery>> &ObjectTypes)
{
	ViewFrustumRecorder->RecordViewFrustum(ViewFrustum);
	bOnlyOverlapWithHelps = OnlyWithHelps;
	ObjectTypesToOverlap = ObjectTypes;
}
