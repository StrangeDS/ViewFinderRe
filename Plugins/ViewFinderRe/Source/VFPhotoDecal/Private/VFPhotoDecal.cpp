#include "VFPhotoDecal.h"

#include "Engine/Texture2D.h"
#include "Components/DecalComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "VFLog.h"
#include "VFPhotoCaptureComponent.h"
#include "VFViewFrustumComponent.h"
#include "VFPhotoDecalDeveloperSettings.h"
#include "VFHelperComponent.h"

AVFPhotoDecal::AVFPhotoDecal()
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    CaptureRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CaptureRoot"));
    CaptureRoot->SetupAttachment(RootComponent);

    CaptureOfDecal = CreateDefaultSubobject<UVFPhotoCaptureComponent>(TEXT("CaptureOfDecal"));
    CaptureOfDecal->SetupAttachment(CaptureRoot);

    CaptureOfDepth = CreateDefaultSubobject<UVFPhotoCaptureComponent>(TEXT("CaptureOfDepth"));
    CaptureOfDepth->SetupAttachment(CaptureRoot);
    CaptureOfDepth->CaptureSource = ESceneCaptureSource::SCS_SceneDepth;
    CaptureOfDepth->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;

    ViewFrustum = CreateDefaultSubobject<UVFViewFrustumComponent>(TEXT("ViewFrustum"));
    ViewFrustum->SetupAttachment(CaptureRoot);

    Decal = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
    Decal->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialSelector(
        TEXT("/ViewFinderRe/Materials/Decal/MI_Decal_Photolize.MI_Decal_Photolize"));
    Matirial = MaterialSelector.Object;
    Decal->SetDecalMaterial(Matirial);
    Decal->SetHiddenInGame(true);

    Helper = CreateDefaultSubobject<UVFHelperComponent>("Helper");
    Helper->ShowInPhotoRule = FVFShowInPhotoRule::OriginalOnly;
    Helper->bCanBeTakenInPhoto = false;
    Helper->bCanBePlacedByPhoto = false;
}

void AVFPhotoDecal::OnConstruction(const FTransform &Transform)
{
    Super::OnConstruction(Transform);

    CaptureOfDecal->FOVAngle = ViewAngle;
    CaptureOfDecal->CustomNearClippingPlane = StartDis;
    CaptureOfDecal->MaxViewDistanceOverride = EndDis;
    CaptureOfDepth->FOVAngle = ViewAngle;
    CaptureOfDepth->CustomNearClippingPlane = StartDis;
    CaptureOfDepth->MaxViewDistanceOverride = EndDis;
    AspectRatio = CaptureOfDecal->GetTargetAspectRatio();
    ViewFrustum->RegenerateViewFrustum(ViewAngle, AspectRatio, StartDis, EndDis);
    FVector Scale = Decal->GetRelativeScale3D();
    Decal->SetRelativeScale3D(FVector(Scale.X, Scale.Y, Scale.Y / AspectRatio));
}

void AVFPhotoDecal::DrawDecal(bool ForceToUpdate)
{
    if (bOnlyCatchManagedActors)
    {
        CaptureOfDecal->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
        CaptureOfDecal->ShowOnlyActors = ManagedActors;
    }
    else
    {
        CaptureOfDecal->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
        CaptureOfDecal->ShowOnlyActors.Reset();
    }

    if (GetMaterialInstance())
    {
        MaterialInstance->SetVectorParameterValue(TEXT("CameraPosition"), CaptureRoot->GetComponentLocation());
        MaterialInstance->SetVectorParameterValue(TEXT("CameraFacing"), CaptureRoot->GetComponentRotation().Vector());
        MaterialInstance->SetVectorParameterValue(TEXT("DecalSize"), Decal->DecalSize);
        MaterialInstance->SetScalarParameterValue(TEXT("FOVAngle"), ViewAngle);
        MaterialInstance->SetScalarParameterValue(TEXT("AspectRatio"), AspectRatio);
        MaterialInstance->SetScalarParameterValue(TEXT("LightFix"),
                                                  GetDefault<UVFPhotoDecalDeveloperSettings>()->PhotoDecalLightFix);

        CaptureOfDecal->CaptureScene();
        if (!IsValid(TextureOfDecal))
        {
            TextureOfDecal = CaptureOfDecal->DrawATexture2D();
        }
        else if (ForceToUpdate)
        {
            CaptureOfDecal->DrawOnTexture2D(TextureOfDecal);
        }
        MaterialInstance->SetTextureParameterValue(TEXT("Texture"), TextureOfDecal);
    }
    else
    {
        VF_LOG(Error, TEXT("%s invalid MaterialInstance."), __FUNCTIONW__);
    }
}

void AVFPhotoDecal::DrawSceneDepth(bool ForceToUpdate)
{
    if (GetMaterialInstance())
    {
        CaptureOfDepth->CaptureScene();
        if (!IsValid(TextureOfDepth))
        {
            TextureOfDepth = CaptureOfDepth->DrawATexture2D();
        }
        else if (ForceToUpdate)
        {
            CaptureOfDepth->DrawOnTexture2D(TextureOfDepth);
        }
        MaterialInstance->SetTextureParameterValue(TEXT("TextureOfDepth"), TextureOfDepth);
    }
    else
    {
        VF_LOG(Error, TEXT("%s invalid MaterialInstance."), __FUNCTIONW__);
    }
}

void AVFPhotoDecal::ReplaceWithDecal_Implementation(bool ForceToUpdate)
{
    if (bReplacing)
        return;

    bReplacing = true;
    DrawDecal(ForceToUpdate);
    SetDecalEnabled(bReplacing);
    DrawSceneDepth(ForceToUpdate);

    OnReplace.Broadcast();
}

#if WITH_EDITOR
void AVFPhotoDecal::ReplaceWithDecalInEditor()
{
    ReplaceWithDecal(true);
}
#endif

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

    if (!Enabled)
    {
        PropsMap.Reset();
    }

    for (auto &Actor : ManagedActors)
    {
        if (!IsValid(Actor))
        {
            VF_LOG(Error, TEXT("%s: Invalid Actor in ManagedActors."), __FUNCTIONW__);
            continue;
        }
        Actor->SetActorHiddenInGame(!Enabled);
        Actor->SetActorEnableCollision(Enabled);

        if (!Enabled)
        {
            TArray<UPrimitiveComponent *> Comps;
            Actor->GetComponents<UPrimitiveComponent>(Comps, false);
            for (auto Comp : Comps)
            {
                FVFPhotoDecalRecordProps Props{
                    Comp->BodyInstance.bSimulatePhysics,
                    Comp->IsGravityEnabled()};
                PropsMap.Add(Comp, Props);
                Comp->SetSimulatePhysics(false);
                Comp->BodyInstance.bEnableGravity = false;
            }
        }
    }

    if (Enabled)
    {
        for (auto &[Comp, Props] : PropsMap)
        {
            if (ensureMsgf(IsValid(Comp), TEXT("%s Invalid Comp."), __FUNCTIONW__))
            {
                Comp->SetSimulatePhysics(Props.bSimulatePhysics);
                Comp->BodyInstance.bEnableGravity = Props.bGravity;
            }
        }
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
    if (!IsValid(MaterialInstance))
        MaterialInstance = Decal->CreateDynamicMaterialInstance();
    return MaterialInstance;
}

UVFHelperComponent *AVFPhotoDecal::GetHelper_Implementation()
{
    return Helper;
}
