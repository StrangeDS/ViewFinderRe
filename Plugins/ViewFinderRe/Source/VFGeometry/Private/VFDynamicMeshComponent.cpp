// Copyright StrangeDS. All Rights Reserved.

#include "VFDynamicMeshComponent.h"

#include "Engine/World.h"
#include "TimerManager.h"

#include "VFLog.h"
#include "VFGeometryFunctions.h"
#include "VFGeometryDeveloperSettings.h"

FVFDMCompRecordProps FVFDMCompRecordProps::GenerateNew()
{
    FVFDMCompRecordProps New(*this);
    New.LevelOfCollision = FMath::Clamp(LevelOfCollision + 1, 0, 255);
    return New;
}

void FVFDMCompRecordProps::Reset()
{
    bSimulatePhysicsRecorder = false;
    bEnableGravityRecorder = false;
    bCastShadowRecorder = false;
    LevelOfCollision = 0;
}

UVFDynamicMeshComponent::UVFDynamicMeshComponent(const FObjectInitializer &ObjectInitializer)
    : UDynamicMeshComponent(ObjectInitializer)
{
    SetMobility(EComponentMobility::Movable);
}

void UVFDynamicMeshComponent::BeginPlay()
{
    Super::BeginPlay();

    if (UVFGeometryFunctions::IsEditorCreated(this))
    {
        // Delay one frame to avoid order issues with BeginPlay in other locations during rewinding
        GetWorld()->GetTimerManager().SetTimerForNextTick(
            [this]()
            {
                Init(Cast<UPrimitiveComponent>(GetAttachParent()));
            });
    }
}

void UVFDynamicMeshComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Clear();

    Super::EndPlay(EndPlayReason);
}

void UVFDynamicMeshComponent::Init(UPrimitiveComponent *Source)
{
    check(Source);
    SourceComponent = Source;

    /*
    During the copying actors process in photo-taking workflow, UVFDynamicMeshComponents will be disassembled (Actor copying), then reassembled.
    Errors will occur if it's a root component. See UVFFunctions::CloneActorRuntime() for details.
    */
    ensureAlwaysMsgf(GetOwner()->GetRootComponent() != this,
                     TEXT("%s is RootComponent in %s."),
                     *GetName(), *GetOwner()->GetName());

    // Components obtained from the pool have inconsistent properties and need manual synchronization
    if (auto DMComp = Cast<UVFDynamicMeshComponent>(Source))
    {
        bEnabled = DMComp->bEnabled;
        Props = DMComp->Props.GenerateNew();
    }
}

void UVFDynamicMeshComponent::Clear()
{
    SourceComponent = nullptr;

    Props.Reset();
    MeshObject->Reset();
    ClearSimpleCollisionShapes(true);
}

void UVFDynamicMeshComponent::CopyMeshFromComponent(UPrimitiveComponent *Source)
{
    UVFGeometryFunctions::CopyMeshFromComponent(
        Source,
        MeshObject,
        GetDefault<UVFGeometryDeveloperSettings>()->CopyMeshOption,
        false);

    // About Physics
    SetCollisionProfileName(Source->GetCollisionProfileName());
    if (auto SourceVFDMComp = GetSourceVFDMComp())
    {
        SetComplexAsSimpleCollisionEnabled(SourceVFDMComp->bEnableComplexCollision, true);
    }
    else
    {
        Props.bSimulatePhysicsRecorder = Source->IsSimulatingPhysics();
        Props.bEnableGravityRecorder = Source->IsGravityEnabled();
        Props.bCastShadowRecorder = Source->CastShadow;
        bool UseSimpleCollision = Props.bUseSimpleCollision || Props.bSimulatePhysicsRecorder;
        SetComplexAsSimpleCollisionEnabled(!UseSimpleCollision, true);
    }
    UpdateSimpleCollision();
    SetCollisionEnabled(Source->GetCollisionEnabled());

    UpdateMaterials();

    bRenderCustomDepth = Source->bRenderCustomDepth;
    CustomDepthStencilValue = Source->CustomDepthStencilValue;
}

void UVFDynamicMeshComponent::ReplaceMeshForComponent(UPrimitiveComponent *Source)
{
    CopyMeshFromComponent(Source);

    Source->SetSimulatePhysics(false);
    Source->SetCollisionProfileName("NoCollision");
    Source->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Source->SetHiddenInGame(true);

    SetSimulatePhysics(Props.bSimulatePhysicsRecorder);
    SetEnableGravity(Props.bEnableGravityRecorder);
    SetCastShadow(Props.bCastShadowRecorder);
}

void UVFDynamicMeshComponent::IntersectMeshWithDMComp(UDynamicMeshComponent *Tool)
{
    static EVF_GeometryScriptBooleanOperation Operation =
        EVF_GeometryScriptBooleanOperation::Intersection;

    const FVF_GeometryScriptMeshBooleanOptions &Options =
        GetDefault<UVFGeometryDeveloperSettings>()->IntersectOption;
    UVFGeometryFunctions::ApplyMeshBoolean(
        MeshObject,
        GetComponentToWorld(),
        Tool->GetDynamicMesh(),
        Tool->GetComponentToWorld(),
        Operation,
        Options);
    UpdateSimpleCollision();
}

void UVFDynamicMeshComponent::SubtractMeshWithDMComp(UDynamicMeshComponent *Tool)
{
    static EVF_GeometryScriptBooleanOperation Operation =
        EVF_GeometryScriptBooleanOperation::Subtract;

    const FVF_GeometryScriptMeshBooleanOptions &Options =
        GetDefault<UVFGeometryDeveloperSettings>()->SubtractOption;
    UVFGeometryFunctions::ApplyMeshBoolean(
        MeshObject,
        GetComponentToWorld(),
        Tool->GetDynamicMesh(),
        Tool->GetComponentToWorld(),
        Operation,
        Options);
    UpdateSimpleCollision();
}

void UVFDynamicMeshComponent::UnionMeshWithDMComp(UDynamicMeshComponent *Tool)
{
    static EVF_GeometryScriptBooleanOperation Operation =
        EVF_GeometryScriptBooleanOperation::Union;

    const FVF_GeometryScriptMeshBooleanOptions &Options =
        GetDefault<UVFGeometryDeveloperSettings>()->UnionOption;
    UVFGeometryFunctions::ApplyMeshBoolean(
        MeshObject,
        GetComponentToWorld(),
        Tool->GetDynamicMesh(),
        Tool->GetComponentToWorld(),
        Operation,
        Options);
    UpdateSimpleCollision();
}

void UVFDynamicMeshComponent::UpdateSimpleCollision()
{
    if (bEnableComplexCollision)
        return;

    UVFGeometryFunctions::SetDynamicMeshCollisionFromMesh(
        MeshObject,
        this,
        GetDefault<UVFGeometryDeveloperSettings>()->GetCollisionOption(Props.LevelOfCollision));
}

#if WITH_EDITOR

void UVFDynamicMeshComponent::DrawSimpleShapesCollision()
{
    FKAggregateGeom KAggregateGeom = GetSimpleCollisionShapes();
    auto SphereElems = KAggregateGeom.SphereElems;
    auto BoxElems = KAggregateGeom.BoxElems;
    auto SphylElems = KAggregateGeom.SphylElems;
    VF_LOG(Log, TEXT("SimpleShapes.Num(): %i"),
           SphereElems.Num() + BoxElems.Num() + SphylElems.Num());

    FTransform Transform = GetComponentTransform();
    UWorld *World = GetWorld();
    FColor Color(150, 100, 0, 50);
    float Time = 5.0f;
    for (FKSphereElem Elem : SphereElems)
    {
        DrawDebugSphere(World,
                        Transform.TransformPosition(Elem.Center),
                        Elem.Radius,
                        4,
                        Color,
                        false,
                        Time);
    }
    for (FKBoxElem Elem : BoxElems)
    {
        DrawDebugBox(World,
                     Transform.TransformPosition(Elem.Center),
                     FVector(Elem.X, Elem.Y, Elem.Z),
                     Transform.TransformRotation(Elem.Rotation.Quaternion()),
                     Color,
                     false,
                     Time);
    }
    for (FKSphylElem Elem : SphylElems)
    {
        DrawDebugCapsule(World,
                         Transform.TransformPosition(Elem.Center),
                         Elem.Length,
                         Elem.Radius,
                         Transform.TransformRotation(Elem.Rotation.Quaternion()),
                         Color,
                         false,
                         Time);
    }
}

void UVFDynamicMeshComponent::DrawConvexCollision()
{
    FKAggregateGeom KAggregateGeom = GetSimpleCollisionShapes();
    auto ConvexElems = KAggregateGeom.ConvexElems;
    FTransform Transform = GetComponentTransform();

    VF_LOG(Log, TEXT("ConvexElems.Num(): %i"), ConvexElems.Num());
    for (auto Elem : ConvexElems)
    {
        auto VertexData = Elem.VertexData;
        VF_LOG(Log, TEXT("VertexData.Num(): %i"), VertexData.Num());
        for (int i = 0; i < VertexData.Num(); ++i)
        {
            VertexData[i] = Transform.TransformPosition(VertexData[i]);
        }
        DrawDebugMesh(
            GetWorld(),
            VertexData,
            Elem.IndexData,
            FColor(150, 100, 0, 50),
            false, 5.0f, SDPG_Foreground);
    }
}
#endif

void UVFDynamicMeshComponent::UpdateMaterials()
{
    for (int i = 0; i < SourceComponent->GetNumMaterials(); i++)
    {
        SetMaterial(i, SourceComponent->GetMaterial(i));
    }
}

void UVFDynamicMeshComponent::SetEnabled(bool Enabled)
{
    if (bEnabled == Enabled)
        return;
    bEnabled = Enabled;

    if (bEnabled)
    {
        SetSimulatePhysics(Props.bSimulatePhysicsRecorder);
        SetEnableGravity(Props.bEnableGravityRecorder);
    }
    else
    {
        SetSimulatePhysics(false);
        SetEnableGravity(false);
        if (Props.bSimulatePhysicsRecorder)
            AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
    }
}

bool UVFDynamicMeshComponent::IsEnabled()
{
    return bEnabled;
}

UPrimitiveComponent *UVFDynamicMeshComponent::GetSourceComponent()
{
    return SourceComponent;
}

UVFDynamicMeshComponent *UVFDynamicMeshComponent::GetSourceVFDMComp()
{
    if (!IsValid(SourceComponent))
        return nullptr;
    return Cast<UVFDynamicMeshComponent>(SourceComponent);
}

void UVFDynamicMeshComponent::RestoreSourceComponent()
{
    if (!IsValid(SourceComponent))
        return;

    SourceComponent->SetHiddenInGame(false);
    SourceComponent->SetCollisionProfileName(GetCollisionProfileName());
    SourceComponent->SetCollisionEnabled(GetCollisionEnabled());
    SourceComponent->SetSimulatePhysics(Props.bSimulatePhysicsRecorder);
    SourceComponent->SetEnableGravity(Props.bEnableGravityRecorder);
}

#if WITH_EDITOR
void UVFDynamicMeshComponent::ReplaceMeshForComponentInEditor()
{
    const FScopedTransaction Transaction(FText::FromString("ReplaceMeshForComponentInEditor"));

    auto Source = Cast<UPrimitiveComponent>(GetAttachParent());
    if (!IsValid(Source))
        VF_LOG(Error, TEXT("%s GetAttachParent() is not a UPrimitiveComponent."), __FUNCTIONW__);

    Modify();
    SourceComponent = Source;
    ReplaceMeshForComponent(Source);
    SourceComponent = nullptr;
    VF_LOG(Log, TEXT("%s has Replaced parent Component."), *GetName());
}

void UVFDynamicMeshComponent::RestoreSourceComponentInEditor()
{
    const FScopedTransaction Transaction(FText::FromString("RestoreSourceComponentInEditor"));
    Modify();

    if (!Props.bSimulatePhysicsRecorder)
    {
        auto Source = Cast<UPrimitiveComponent>(GetAttachParent());
        if (!IsValid(Source))
        {
            VF_LOG(Error, TEXT("%s GetAttachParent() is not a UPrimitiveComponent."), __FUNCTIONW__);
            return;
        }
        SourceComponent = Source;
    }
    RestoreSourceComponent();
    SourceComponent = nullptr;
    VF_LOG(Log, TEXT("%s has Restored parent Component."), *GetName());
}
#endif

void UVFDynamicMeshComponent::AfterGet_Implementation()
{
}

void UVFDynamicMeshComponent::BeforeReturn_Implementation()
{
}