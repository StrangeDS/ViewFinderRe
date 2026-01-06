// Copyright 2026, StrangeDS. All Rights Reserved.

#include "VFPawnStandIn.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"

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
		TargetPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	}

	auto Rotator = RootComponent->GetComponentRotation();
	/*
	If we don't reset it to zero, it could actually enable a new gameplay mechanic.
	But then we'd have to consider jump direction and gravity direction,
	and intuitively,
	the gravity for other objects would need to be adjusted accordingly as well.
	*/
	Rotator.Roll = 0.f;
	TargetPawn->SetActorLocation(GetActorLocation());
	TargetPawn->FaceRotation(Rotator);
	if (auto Controller = TargetPawn->GetController())
		Controller->SetControlRotation(Rotator);

	SetActorHiddenInGame(true);
}

void AVFPawnStandIn::Hide(UObject *Sender)
{
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