#include "VFPhoto2D.h"

#include "Engine/Texture2D.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "VFCommon.h"
#include "VFDynamicMeshComponent.h"
#include "VFPhoto3D.h"
#include "VFHelperComponent.h"
#include "VFFunctions.h"
#include "VFPhotoCaptureComponent.h"

AVFPhoto2D::AVFPhoto2D() : Super()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(RootComponent);
	StaticMesh->SetCollisionProfileName(TEXT("IgnoreOnlyPawn"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshSelector(
		TEXT("/ViewFinderRe/StaticMeshes/ST_Photo.ST_Photo"));
	StaticMeshObject = MeshSelector.Object;
	StaticMesh->SetStaticMesh(StaticMeshObject);

	Helper = CreateDefaultSubobject<UVFHelperComponent>("Helper");
}

void AVFPhoto2D::BeginPlay()
{
	Super::BeginPlay();

	Helper->OnOriginalEndTakingPhoto.AddUniqueDynamic(this, &AVFPhoto2D::CopyPhoto3D);
	if (bIsRecursive)
	{
		Helper->OnCopyBeforeBeingEnabled.AddUniqueDynamic(this, &AVFPhoto2D::CopyRecursivePhoto3D);
	}
}

void AVFPhoto2D::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	MaterialInstance = nullptr;
	Texture2D = nullptr;

	Super::EndPlay(EndPlayReason);
}

void AVFPhoto2D::SetPhoto3D(AVFPhoto3D *Photo)
{
	Photo3D = Photo;
}

AVFPhoto3D *AVFPhoto2D::GetPhoto3D()
{
	return Photo3D;
}

void AVFPhoto2D::SetPhoto(UVFPhotoCaptureComponent *PhotoCapture)
{
	if (IsValid(PhotoCapture))
	{
		PhotoCapture->CaptureScene();
		Texture2D = PhotoCapture->DrawATexture2D();
		float Scale = PhotoCapture->FOVAngle / 90.0f;
		StaticMesh->SetRelativeScale3D(FVector(StaticMesh->GetRelativeScale3D().X, Scale, Scale));
	}
	else
	{
		Texture2D = nullptr;
		StaticMesh->SetRelativeScale3D(FVector::OneVector);
	}

	if (GetMaterialInstance())
	{
		if (IsValid(PhotoCapture))
		{
			float AspectRatio = (float)PhotoCapture->TargetWidth / PhotoCapture->TargetHeight;
			MaterialInstance->SetTextureParameterValue(TextureName, Texture2D);
			MaterialInstance->SetScalarParameterValue(RatioName, AspectRatio);
		}
		else
		{
			MaterialInstance->ClearParameterValues();
		}
	}
	else
	{
		VF_LOG(Warning, TEXT("%s  has no MaterialInstance."), __FUNCTIONW__);
		return;
	}
}

void AVFPhoto2D::FoldUp()
{
	if (!IsValid(Photo3D))
		return;

	if (State == EVFPhoto2DState::Folded)
		return;
	State = EVFPhoto2DState::Folded;

	Photo3D->FoldUp();
}

void AVFPhoto2D::Preview(const FTransform &WorldTrans, const bool &Enabled)
{
	if (!IsValid(Photo3D))
		return;

	if (Enabled)
	{
		Photo3D->SetActorTransform(WorldTrans);
	}
	Photo3D->SetViewFrustumVisible(Enabled);
}

void AVFPhoto2D::PlaceDown()
{
	if (!IsValid(Photo3D))
		return;

	if (State == EVFPhoto2DState::Placed)
		return;
	State = EVFPhoto2DState::Placed;

	Photo3D->SetActorTransform(FTransform(GetActorRotation(), GetActorLocation()));
	Photo3D->PlaceDown();
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void AVFPhoto2D::CopyPhoto3D(UObject *Sender)
{
	if (!IsValid(Photo3D))
	{
		VF_LOG(Warning, TEXT("%s invalid Photo3D."), __FUNCTIONW__);
		return;
	}

	auto Photo3DNew = UVFFunctions::CloneActorRuntimeRecursive<AVFPhoto3D>(Photo3D);
	Photo3DNew->RecordProperty(Photo3D->ViewFrustumRecorder,
							   Photo3D->bOnlyOverlapWithHelps,
							   Photo3D->ObjectTypesToOverlap);
	Photo3D = Photo3DNew;
}

UMaterialInstanceDynamic *AVFPhoto2D::GetMaterialInstance_Implementation()
{
	if (!IsValid(MaterialInstance))
		MaterialInstance = StaticMesh->CreateAndSetMaterialInstanceDynamic(0);
	return MaterialInstance;
}

bool AVFPhoto2D::ReattachToComponent(USceneComponent *Target)
{
	if (IsValid(Target))
	{
		if (Target == GetRootComponent()->GetAttachParent())
			return false;
		AttachToComponent(Target, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	else
	{
		if (!GetAttachParentActor())
			return false;
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
	return true;
}

void AVFPhoto2D::CopyRecursivePhoto3D(UObject *Sender)
{
	auto Photo3DOuter = Cast<AVFPhoto3D>(Sender);
	if (!IsValid(Photo3DOuter))
	{
		VF_LOG(Warning, TEXT("%s invalid Photo3D."), __FUNCTIONW__);
		return;
	}

	auto Photo3DNew = UVFFunctions::CloneActorRuntimeRecursive<AVFPhoto3D>(Photo3DOuter);
	Photo3DNew->RecordProperty(Photo3DOuter->ViewFrustumRecorder,
							   Photo3DOuter->bOnlyOverlapWithHelps,
							   Photo3DOuter->ObjectTypesToOverlap);
	SetPhoto3D(Photo3DNew);
	FoldUp();
}

UVFHelperComponent *AVFPhoto2D::GetHelper_Implementation()
{
	return Helper;
}