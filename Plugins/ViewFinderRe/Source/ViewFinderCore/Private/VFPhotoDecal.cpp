#include "VFPhotoDecal.h"

#include "Components/DecalComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "VFPhotoCaptureComponent.h"

AVFPhotoDecal::AVFPhotoDecal()
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    PhotoCapture = CreateDefaultSubobject<UVFPhotoCaptureComponent>(TEXT("PhotoCapture"));
    PhotoCapture->SetupAttachment(RootComponent);
    PhotoCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;

    Decal = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
    Decal->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialSelector(
        TEXT("/ViewFinderRe/Content/Materials/Decal/MDI_Photolize.MDI_Photolize"));
    Matirial = MaterialSelector.Object;
    Decal->SetDecalMaterial(Matirial);
    Decal->SetHiddenInGame(true);
}

void AVFPhotoDecal::BeginPlay()
{
    // 只记录第一级的子Actor
    ManagedActors.Reset();
    GetAttachedActors(ManagedActors, false, false);

    TArray<AActor *> ShowActors = ManagedActors;
    for (auto &Actor : ManagedActors)
    {
        Actor->GetAttachedActors(ShowActors, false, true);
    }
    PhotoCapture->ShowOnlyActors = ShowActors;

    MaterialInstance = Decal->CreateDynamicMaterialInstance();
    PhotoCapture->Init(MaterialInstance);

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
