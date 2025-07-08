#include "VFPlaneActor.h"

#if WITH_EDITOR
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#endif

#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"

#include "VFPhotoCatcher.h"
#include "Subsystems/VFBackgroundWorldSubsystem.h"

AVFPlaneActor::AVFPlaneActor()
{
	PrimaryActorTick.bCanEverTick = false;

	TransformRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = TransformRoot;

	Plane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plane"));
	Plane->SetupAttachment(RootComponent);
	Plane->SetRelativeRotation(FRotator(0.f, 90.f, 90.f));
	Plane->CastShadow = false;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshSelector(
		TEXT("/ViewFinderRe/StaticMeshes/SM_Plane.SM_Plane"));
	Plane->SetStaticMesh(MeshSelector.Object);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialSelector(
		TEXT("/ViewFinderRe/Materials/Background/MI_Background.MI_Background"));
	Plane->SetMaterial(0, MaterialSelector.Object);

	Helper = CreateDefaultSubobject<UVFHelperComponent>("Helper");
	Helper->bCanShowInPhoto = false;
}

void AVFPlaneActor::BeginPlay()
{
	Super::BeginPlay();

	Helper->OnOriginalEndTakingPhoto.AddUniqueDynamic(this, &AVFPlaneActor::HandleEndTakingPhoto);
	Helper->OnCopyEndPlacingPhoto.AddUniqueDynamic(this, &AVFPlaneActor::HandleEndPlacingPhoto);

	if (auto BackgroundSystem = GetWorld()->GetSubsystem<UVFBackgroundWorldSubsystem>())
	{
		BackgroundSystem->Register(this);
	}
}

void AVFPlaneActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (auto BackgroundSystem = GetWorld()->GetSubsystem<UVFBackgroundWorldSubsystem>())
	{
		BackgroundSystem->Unregister(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AVFPlaneActor::SetPlane(FVector Location, FVector Direction, float Width, float Height)
{
	SetActorTransform(FTransform(Direction.ToOrientationQuat(),
								 Location,
								 FVector(Width / 100.0f, Height / 100.0f, 1.0f)));
}

void AVFPlaneActor::SetPlaneMaterial(UTexture2D *Texture)
{
	auto MaterialInstance = Plane->CreateAndSetMaterialInstanceDynamic(0);
	MaterialInstance->SetTextureParameterValue(TEXT("Texture"), Texture);
}

void AVFPlaneActor::HandleEndTakingPhoto(UObject *Sender)
{
	if (bDestroyAfterTakingPhoto)
	{
		Destroy();
	}
}

void AVFPlaneActor::HandleEndPlacingPhoto(UObject *Sender)
{
	bDestroyAfterTakingPhoto = false;
	Helper->bCanShowInPhoto = true;
	Helper->bCanBeTakenInPhoto = false;
}

#if WITH_EDITOR
void AVFPlaneActor::FaceToPawn()
{
	APlayerController *PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	APawn *Pawn = PlayerController->GetPawn();
	auto Camera = Pawn->GetComponentByClass<UCameraComponent>();
	auto Location = Pawn->GetControlRotation().RotateVector(FVector::ForwardVector * 2000.0f);
	auto Direction = (Location - Camera->GetComponentLocation()).GetSafeNormal();
	SetPlane(Location, Direction, 2000.0f, 3000.0f);
}
#endif

UVFHelperComponent *AVFPlaneActor::GetHelper_Implementation()
{
	return Helper;
}