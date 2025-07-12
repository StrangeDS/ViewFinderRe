#include "VFPawnStandIn.h"

#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

#include "VFPhoto3D.h"
#include "VFPhotoCatcher.h"
#include "VFDynamicMeshComponent.h"
#include "VFHelperComponent.h"

AVFPawnStandIn::AVFPawnStandIn() : Super()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMesh->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshSelector(
		TEXT("/ViewFinderRe/StaticMeshes/SM_PawnStandIn.SM_PawnStandIn"));
	StaticMeshObject = MeshSelector.Object;
	StaticMesh->SetStaticMesh(StaticMeshObject);
	StaticMesh->SetCollisionProfileName(TEXT("NoCollision"));

	DynamicMesh = CreateDefaultSubobject<UVFDynamicMeshComponent>("DynamicMesh");
	DynamicMesh->SetupAttachment(StaticMesh);
	DynamicMesh->SetCollisionProfileName(TEXT("NoCollision"));

	Helper = CreateDefaultSubobject<UVFHelperComponent>("Helper");
	Helper->bCanBeTakenInPhoto = true;
	Helper->bCanBePlacedByPhoto = false;
}

void AVFPawnStandIn::BeginPlay()
{
	Super::BeginPlay();

	Helper->OnOriginalEndTakingPhoto.AddUniqueDynamic(this, &AVFPawnStandIn::Hide);
	Helper->OnCopyEndPlacingPhoto.AddUniqueDynamic(this, &AVFPawnStandIn::TeleportTargetPawn);
}

void AVFPawnStandIn::SetTargetPawn(APawn *Pawn)
{
	check(Pawn);
	TargetPawn = Pawn;
}

void AVFPawnStandIn::TeleportTargetPawn(UObject *Sender)
{
	if (!ensure(IsValid(TargetPawn)))
	{
		TargetPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	}

	auto Rotator = RootComponent->GetComponentRotation();
	Rotator.Roll = 0.f; // 不进行置零的话, 倒也是个新玩法. 但又得考虑跳跃方向和重力方向, 其它物体的重力也得一同改变才符合直觉.
	TargetPawn->SetActorLocation(GetActorLocation());
	TargetPawn->FaceRotation(Rotator);
	if (auto Controller = TargetPawn->GetController())
		Controller->SetControlRotation(Rotator);

	SetActorHiddenInGame(true);
}

void AVFPawnStandIn::Hide(UObject *Sender)
{
	auto PhotoCathcer = Cast<AVFPhotoCatcher>(Sender);
	SetActorHiddenInGame(true);
}

void AVFPawnStandIn::SetOriginalActor_Implementation(AActor *Original)
{
	IVFStandInInterface::SetOriginalActor_Implementation(Original);

	SetTargetPawn(Cast<APawn>(Original));
}

UPrimitiveComponent *AVFPawnStandIn::GetPrimitiveComp_Implementation()
{
	return DynamicMesh;
}

UVFHelperComponent *AVFPawnStandIn::GetHelper_Implementation()
{
	return Helper;
}