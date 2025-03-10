#include "VFPhotoDecal.h"

#include "Components/DecalComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

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
        TEXT("/ViewFinderRe/Content/Materials/Decal/MDI_Photolize.MDI_Photolize"));
    Matirial = MaterialSelector.Object;
    Decal->SetDecalMaterial(Matirial);
    Decal->SetHiddenInGame(true);
}

void AVFPhotoDecal::OnConstruction(const FTransform &Transform)
{
    Super::OnConstruction(Transform);

    PhotoCapture->FOVAngle = ViewAngle;
    ViewFrustum->RegenerateViewFrustum(ViewAngle, AspectRatio, StartDis, EndDis);
}

void AVFPhotoDecal::BeginPlay()
{
    MaterialInstance = Decal->CreateDynamicMaterialInstance();
    PhotoCapture->Init(MaterialInstance);

    ManagedActors.Reset();
    GetAttachedActors(ManagedActors, true, true);

    // 蓝图的BeginPlay需要在MaterialInstance被创建后
    Super::BeginPlay();
}

void AVFPhotoDecal::DrawDecal()
{
    if (ensure(MaterialInstance))
    {
        MaterialInstance->SetVectorParameterValue(TEXT("CameraPosition"), PhotoCapture->GetComponentLocation());
        MaterialInstance->SetVectorParameterValue(TEXT("CameraFacing"), PhotoCapture->GetComponentRotation().Vector());
        MaterialInstance->SetScalarParameterValue(TEXT("FOVAngle"), PhotoCapture->FOVAngle);
        MaterialInstance->SetScalarParameterValue(TEXT("YofDecal"), Decal->DecalSize.Y);
        MaterialInstance->SetScalarParameterValue(TEXT("AspectRatio"), AspectRatio);
    }

    PhotoCapture->DrawAFrame();
}

void AVFPhotoDecal::Replace()
{
    if (bReplacing)
        return;

    bReplacing = true;
    DrawDecal();
    SetDecalEnabled(bReplacing);

    OnReplace.Broadcast();
}

void AVFPhotoDecal::Restore()
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
    for (auto &Actor : ManagedActors)
    {
        Actor->SetActorHiddenInGame(!Enabled);
        Actor->SetActorEnableCollision(Enabled);
    }
}
