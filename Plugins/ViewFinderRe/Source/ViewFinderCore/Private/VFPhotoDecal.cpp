#include "VFPhotoDecal.h"

#include "Components/DecalComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "VFCommon.h"
#include "VFPhotoCaptureComponent.h"
#include "VFViewFrustumComponent.h"

AVFPhotoDecal::AVFPhotoDecal()
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    PhotoCapture = CreateDefaultSubobject<UVFPhotoCaptureComponent>(TEXT("PhotoCapture"));
    PhotoCapture->SetupAttachment(RootComponent);

    ViewFrustum = CreateDefaultSubobject<UVFViewFrustumComponent>(TEXT("ViewFrustum"));
    ViewFrustum->SetupAttachment(PhotoCapture);

    Decal = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
    Decal->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialSelector(
        TEXT("/ViewFinderRe/Materials/Decal/MDI_Photolize.MDI_Photolize"));
    Matirial = MaterialSelector.Object;
    Decal->SetDecalMaterial(Matirial);
    Decal->SetHiddenInGame(true);
}

void AVFPhotoDecal::OnConstruction(const FTransform &Transform)
{
    Super::OnConstruction(Transform);

    PhotoCapture->FOVAngle = ViewAngle;
    float AspectRatio = PhotoCapture->GetTargetAspectRatio();
    ViewFrustum->RegenerateViewFrustum(ViewAngle, AspectRatio, StartDis, EndDis);
    FVector Scale = Decal->GetRelativeScale3D();
    Decal->SetRelativeScale3D(FVector(Scale.X, Scale.Y, Scale.Y / AspectRatio));
}

// void AVFPhotoDecal::BeginPlay()
// {
//     Super::BeginPlay();
// }

void AVFPhotoDecal::DrawDecal()
{
    if (bOnlyCatchManagedActors)
    {
        PhotoCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
        PhotoCapture->ShowOnlyActors = ManagedActors;
    }
    else
    {
        PhotoCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
        PhotoCapture->ShowOnlyActors.Reset();
    }

    if (GetMaterialInstance())
    {
        MaterialInstance->SetVectorParameterValue(TEXT("CameraPosition"), PhotoCapture->GetComponentLocation());
        MaterialInstance->SetVectorParameterValue(TEXT("CameraFacing"), PhotoCapture->GetComponentRotation().Vector());
        MaterialInstance->SetVectorParameterValue(TEXT("DecalSize"), Decal->DecalSize);
        MaterialInstance->SetScalarParameterValue(TEXT("FOVAngle"), PhotoCapture->FOVAngle);
        MaterialInstance->SetScalarParameterValue(TEXT("AspectRatio"), PhotoCapture->GetTargetAspectRatio());

        PhotoCapture->CaptureScene();
        Texture2D = PhotoCapture->DrawATexture2D();
        MaterialInstance->SetTextureParameterValue(TEXT("Texture"), Texture2D);
    }
    else
    {
        VF_LOG(Warning, TEXT("%s invalid MaterialInstance."), __FUNCTIONW__);
    }
}

void AVFPhotoDecal::ReplaceWithDecal_Implementation()
{
    if (bReplacing)
        return;

    bReplacing = true;
    DrawDecal();
    SetDecalEnabled(bReplacing);

    OnReplace.Broadcast();
}

void AVFPhotoDecal::RestoreWithActors_Implementation()
{
    if (!bReplacing)
        return;

    bReplacing = false;
    SetDecalEnabled(bReplacing);

    OnRestore.Broadcast();
}

void AVFPhotoDecal::SetDecalEnabled(bool Enabled)
{
    Decal->SetHiddenInGame(!Enabled);
    SetManagedActorsEnabled(!Enabled);
}

void AVFPhotoDecal::SetManagedActorsEnabled(bool Enabled)
{
    if (ManagedActors.IsEmpty())
    {
        VF_LOG(Warning, TEXT("%s: No Actors has been managed."), __FUNCTIONW__);
    }
    for (auto &Actor : ManagedActors)
    {
        Actor->SetActorHiddenInGame(!Enabled);
        Actor->SetActorEnableCollision(Enabled);
    }
}

#if WITH_EDITOR
#include "Kismet/KismetSystemLibrary.h"

void AVFPhotoDecal::RecollectActorsWithFrustum()
{
    UKismetSystemLibrary::ComponentOverlapActors(
        ViewFrustum,
        ViewFrustum->GetComponentToWorld(),
        {},
        AActor::StaticClass(),
        {this},
        ManagedActors);
}
#endif

UMaterialInstanceDynamic *AVFPhotoDecal::GetMaterialInstance_Implementation()
{
    if (!MaterialInstance)
        MaterialInstance = Decal->CreateDynamicMaterialInstance();
    return MaterialInstance;
}
