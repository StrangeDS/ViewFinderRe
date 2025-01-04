#include "VFPhotoDecal.h"

#include "Components/DecalComponent.h"

#include "VFPhotoCaptureComponent.h"

AVFPhotoDecal::AVFPhotoDecal()
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    PhotoCapture = CreateDefaultSubobject<UVFPhotoCaptureComponent>(TEXT("PhotoCapture"));
    PhotoCapture->SetupAttachment(RootComponent);
    PhotoCapture->bCaptureEveryFrame = false;
    PhotoCapture->bCaptureOnMovement = false;
    PhotoCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;

    Decal = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
    Decal->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialSelector(
        TEXT("/Game/ViewFinder/Material/Decal/MDI_Photolize.MDI_Photolize"));
    Matirial = MaterialSelector.Object;
    Decal->SetDecalMaterial(Matirial);
}

void AVFPhotoDecal::BeginPlay()
{
    Super::BeginPlay();

    // 只记录第一级的子Actor
    ManagedActors.Reset();
    GetAttachedActors(ManagedActors, false, false);

    TArray<AActor *> ShowActors = ManagedActors;
    for (auto &Actor : ManagedActors)
    {
        Actor->GetAttachedActors(ShowActors, false, true);
    }
    PhotoCapture->ShowOnlyActors = ShowActors;
    PhotoCapture->Init(Decal->CreateDynamicMaterialInstance());
}

void AVFPhotoDecal::DrawDecal()
{
    SetDecalEnabled(false);
    PhotoCapture->DrawAFrame();
}

void AVFPhotoDecal::Replace()
{
    DrawDecal();
    SetDecalEnabled(true);

    OnReplace.Broadcast();
}

void AVFPhotoDecal::Restore()
{
    SetDecalEnabled(false);

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
