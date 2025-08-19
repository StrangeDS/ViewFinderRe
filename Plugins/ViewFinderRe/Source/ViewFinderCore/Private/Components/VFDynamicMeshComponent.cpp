#include "VFDynamicMeshComponent.h"

#include "VFCommon.h"
#include "VFDMCompPoolWorldSubsystem.h"
#include "VFGeometryFunctions.h"
#include "VFFunctions.h"

UVFDynamicMeshComponent::UVFDynamicMeshComponent(const FObjectInitializer &ObjectInitializer)
    : UDynamicMeshComponent(ObjectInitializer)
{
    SetMobility(EComponentMobility::Movable);
}

void UVFDynamicMeshComponent::BeginPlay()
{
    Super::BeginPlay();

    if (UVFFunctions::IsEditorCreated(this))
    {
        // 延迟一帧, 避免回溯时, 与其它地方BeginPlay的顺序问题
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
    在拍照流程的复制过程中, 组件会被拆卸(复制Actor), 然后重新组装.
    若为根组件会出现错误. 详见UVFFunctions::CloneActorRuntime()
    */
    if (GetAttachParent() == nullptr)
    {
        VF_LOG(Warning,
               TEXT("%s is RootComponent in %s."),
               *GetName(), *GetOwner()->GetName());
    }

    /*
    从池中获取到的组件, 属性并不一致, 需要手动同步
    */
    if (auto DMComp = Cast<UVFDynamicMeshComponent>(Source))
    {
        bEnabled = DMComp->bEnabled;
        Props = DMComp->Props;
    }
}

void UVFDynamicMeshComponent::Clear()
{
    SourceComponent = nullptr;

    MeshObject->Reset();
    ClearSimpleCollisionShapes(true);
}

void UVFDynamicMeshComponent::CopyMeshFromComponent(UPrimitiveComponent *Source)
{
    // 复制网格
    UVFGeometryFunctions::CopyMeshFromComponent(
        Source,
        MeshObject,
        FVF_GeometryScriptCopyMeshFromComponentOptions(),
        false);

    // 复制物理
    // 复制碰撞预设, 类型等. 原静态网格体的简单碰撞可能与显示不一致
    // 自动生成的碰撞并不完美, 碰撞的设置也是固定的.
    SetCollisionProfileName(Source->GetCollisionProfileName());
    if (auto SourceVFDMComp = GetSourceVFDMComp())
    {
        SetComplexAsSimpleCollisionEnabled(SourceVFDMComp->bEnableComplexCollision, true);
    }
    else
    {
        bool UseSimpleCollision = Source->BodyInstance.bSimulatePhysics;
        UseSimpleCollision |= !Source->BodyInstance.GetBodySetup() ||
                              Source->BodyInstance.GetBodySetup()->GetCollisionTraceFlag() == ECollisionTraceFlag::CTF_UseSimpleAsComplex;
        SetComplexAsSimpleCollisionEnabled(!UseSimpleCollision, true);
        Props.bSimulatePhysicsRecorder = Source->BodyInstance.bSimulatePhysics;
        Props.bEnableGravityRecorder = Source->IsGravityEnabled();
        Props.bCastShadowRecorder = Source->CastShadow;
    }
    UpdateSimlpeCollision();
    SetCollisionEnabled(Source->GetCollisionEnabled());

    UpdateMaterials();

    bRenderCustomDepth = Source->bRenderCustomDepth;
    CustomDepthStencilValue = Source->CustomDepthStencilValue;

    // TODO: 传递事件. 暂使用Actor接口
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
    static FVF_GeometryScriptMeshBooleanOptions Options;
    static EVF_GeometryScriptBooleanOperation Operation =
        EVF_GeometryScriptBooleanOperation::Intersection;
    UVFGeometryFunctions::ApplyMeshBoolean(
        MeshObject,
        GetComponentToWorld(),
        Tool->GetDynamicMesh(),
        Tool->GetComponentToWorld(),
        Operation,
        Options);
    UpdateSimlpeCollision();
}

void UVFDynamicMeshComponent::SubtractMeshWithDMComp(UDynamicMeshComponent *Tool)
{
    static FVF_GeometryScriptMeshBooleanOptions Options;
    static EVF_GeometryScriptBooleanOperation Operation =
        EVF_GeometryScriptBooleanOperation::Subtract;
    UVFGeometryFunctions::ApplyMeshBoolean(
        MeshObject,
        GetComponentToWorld(),
        Tool->GetDynamicMesh(),
        Tool->GetComponentToWorld(),
        Operation,
        Options);
    UpdateSimlpeCollision();
}

void UVFDynamicMeshComponent::UnionMeshWithDMComp(UDynamicMeshComponent *Tool)
{
    static FVF_GeometryScriptMeshBooleanOptions Options;
    static EVF_GeometryScriptBooleanOperation Operation =
        EVF_GeometryScriptBooleanOperation::Union;
    UVFGeometryFunctions::ApplyMeshBoolean(
        MeshObject,
        GetComponentToWorld(),
        Tool->GetDynamicMesh(),
        Tool->GetComponentToWorld(),
        Operation,
        Options);
    UpdateSimlpeCollision();
}

void UVFDynamicMeshComponent::UpdateSimlpeCollision()
{
    if (bEnableComplexCollision)
        return;
    static FVF_GeometryScriptCollisionFromMeshOptions Options;
    UVFGeometryFunctions::SetDynamicMeshCollisionFromMesh(
        MeshObject,
        this,
        Options);
}

#if WITH_EDITOR
#include "VFCommon.h"

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
    SourceComponent->BodyInstance.bEnableGravity = Props.bEnableGravityRecorder;
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

    auto Source = Cast<UPrimitiveComponent>(GetAttachParent());
    if (!IsValid(Source))
        VF_LOG(Error, TEXT("%s GetAttachParent() is not a UPrimitiveComponent."), __FUNCTIONW__);

    Modify();
    SourceComponent = Source;
    RestoreSourceComponent();
    SourceComponent = nullptr;
    VF_LOG(Log, TEXT("%s has Restored parent Component."), *GetName());
}
#endif