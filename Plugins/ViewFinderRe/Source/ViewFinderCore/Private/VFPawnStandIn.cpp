#include "VFPawnStandIn.h"

#include "Kismet/GameplayStatics.h"

#include "VFDynamicMeshComponent.h"
#include "VFHelperComponent.h"

AVFPawnStandIn::AVFPawnStandIn() : Super()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMesh->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshSelector(
		TEXT("/Game/ViewFinder/StaticMesh/Camera_Temp.Camera_Temp"));
	StaticMeshObject = MeshSelector.Object;
	StaticMesh->SetStaticMesh(StaticMeshObject);

	DynamicMesh = CreateDefaultSubobject<UVFDynamicMeshComponent>("DynamicMesh");
	DynamicMesh->SetupAttachment(RootComponent);
	DynamicMesh->SetCollisionProfileName(TEXT("NoCollision"));

	Helper = CreateDefaultSubobject<UVFHelperComponent>("Helper");
	Helper->bCanBeTakenInPhoto = true;
	Helper->bCanBePlacedByPhoto = false;
}

void AVFPawnStandIn::BeginPlay()
{
	Super::BeginPlay();

	StaticMesh->SetCollisionProfileName(TEXT("NoCollision"));
	DynamicMesh->SetCollisionProfileName(TEXT("NoCollision"));

	Helper->OnOriginalAfterTakingPhoto.AddUniqueDynamic(this, &AVFPawnStandIn::Hide);
	Helper->OnCopyAfterPlacedByPhoto.AddUniqueDynamic(this, &AVFPawnStandIn::TeleportTargetPawn);
}

void AVFPawnStandIn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AVFPawnStandIn::BeginDestroy()
{
	Super::BeginDestroy();
}

void AVFPawnStandIn::SetTargetPawn(APawn *Pawn)
{
	TargetPawn = Pawn;
}

void AVFPawnStandIn::TeleportTargetPawn()
{
	if (!TargetPawn)
	{
		TargetPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	}

	TargetPawn->SetActorLocation(GetActorLocation());
	TargetPawn->FaceRotation(GetActorRotation());
	SetActorHiddenInGame(true);
}

void AVFPawnStandIn::Hide()
{
	SetActorHiddenInGame(true);
}
