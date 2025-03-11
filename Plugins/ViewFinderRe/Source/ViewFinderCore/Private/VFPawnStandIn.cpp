#include "VFPawnStandIn.h"

#include "Kismet/GameplayStatics.h"

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
		TEXT("/ViewFinderRe/StaticMeshes/Camera_Temp.Camera_Temp"));
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

	Helper->OnOriginalBeforeCopyingToPhoto.AddUniqueDynamic(this, &AVFPawnStandIn::Hide);
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
	check(Pawn);
	TargetPawn = Pawn;
}

void AVFPawnStandIn::TeleportTargetPawn(UObject* Sender)
{
	if (!TargetPawn)
	{
		TargetPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	}

	auto Photo3D = Cast<AVFPhoto3D>(Sender);
	auto Quat = Photo3D->GetActorQuat() * ViewQuat;
	auto Rotator = Quat.Rotator();
	Rotator.Roll = 0.f;	// 不进行置零的话, 倒也是个新玩法. 但又得考虑跳跃方向和重力方向, 其它物体的重力也得一同改变才符合直觉.
	TargetPawn->SetActorLocation(GetActorLocation());
	TargetPawn->FaceRotation(Rotator);
	if (auto Controller = TargetPawn->GetController())
		Controller->SetControlRotation(Rotator);

	SetActorHiddenInGame(true);
}

void AVFPawnStandIn::Hide(UObject* Sender)
{
	auto PhotoCathcer = Cast<AVFPhotoCatcher>(Sender);
	ViewQuat = PhotoCathcer->GetFrustumQuat().Inverse() * TargetPawn->GetViewRotation().Quaternion();
	SetActorHiddenInGame(true);
}

void AVFPawnStandIn::SetSourceActor_Implementation(AActor *Source)
{
	IVFStandInInterface::SetSourceActor_Implementation(Source);

	SetTargetPawn(Cast<APawn>(Source));
}

UPrimitiveComponent *AVFPawnStandIn::GetPrimitiveComp_Implementation()
{
	return DynamicMesh;
}