#include "VFDynamicMeshComponent.h"

#include "VFDynamicMeshPoolWorldSubsystem.h"
#include "VFGeometryFunctions.h"

UVFDynamicMeshComponent::UVFDynamicMeshComponent(const FObjectInitializer &ObjectInitializer)
    : UDynamicMeshComponent(ObjectInitializer)
{
    SetMobility(EComponentMobility::Movable);
}

void UVFDynamicMeshComponent::BeginPlay()
{
    Super::BeginPlay();

    MeshPool = GetWorld()->GetSubsystem<UVFDynamicMeshPoolWorldSubsystem>();
}

// void UVFDynamicMeshComponent::SetDynamicMeshFromPool(UDynamicMesh *Mesh, EVFMeshType Type)
// {
//     MeshType = Type;
//     SetDynamicMesh(Mesh);
// }

void UVFDynamicMeshComponent::CopyMeshFromComponent(UPrimitiveComponent *Source)
{
    check(Source);
    SourceComponent = Source;

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
        // 从VFDMComp上复制不需要立即应用, 由SetEnabled启用
        SetComplexAsSimpleCollisionEnabled(SourceVFDMComp->bEnableComplexCollision, true);
        UpdateSimlpeCollision();
        bSimulatePhysicsRecorder = SourceVFDMComp->bSimulatePhysicsRecorder;
        bEnableGravityRecorder = SourceVFDMComp->bEnableGravityRecorder;
    }
    else if (Source->BodyInstance.bSimulatePhysics)
    {
        SetComplexAsSimpleCollisionEnabled(false, true);
        UpdateSimlpeCollision();
        // 从静态网格体上复制物理状态
        bSimulatePhysicsRecorder = Source->BodyInstance.bSimulatePhysics;
        bEnableGravityRecorder = Source->IsGravityEnabled();
    }
    else
    {
        // 使用复杂碰撞
        SetComplexAsSimpleCollisionEnabled(true, true);
    }
    SetCollisionEnabled(Source->GetCollisionEnabled());

    // 复制材质
    SetMaterial(0, Source->GetMaterial(0));

    // TODO: 传递事件. 暂使用Actor接口
}

void UVFDynamicMeshComponent::ReplaceMeshForComponent(UPrimitiveComponent *Source)
{
    CopyMeshFromComponent(Source);
    
    Source->SetSimulatePhysics(false);
    Source->SetCollisionProfileName("NoCollision");
    Source->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Source->SetHiddenInGame(true);

    SetSimulatePhysics(bSimulatePhysicsRecorder);
    SetEnableGravity(bEnableGravityRecorder);
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
    // static FVF_GeometryScriptCollisionFromMeshOptions Options
    // {
    //     false,
    //     EVF_GeometryScriptCollisionGenerationMethod::ConvexHulls,
    //     false,
    //     false,
    //     false,
    //     1.0f,
    //     true,
    //     25,
    //     4,
    //     0.5f,
    //     0.f,
    //     0.1f,
    //     0.1f,
    //     EVF_GeometryScriptSweptHullAxis::Z,
    //     true,
    //     4
    // };
    UVFGeometryFunctions::SetDynamicMeshCollisionFromMesh(
        MeshObject,
        this,
        Options);
}

void UVFDynamicMeshComponent::SetEnabled(bool Enabled)
{
    if (bEnabled == Enabled)
        return;
    bEnabled = Enabled;

    if (bEnabled)
    {
        SetSimulatePhysics(bSimulatePhysicsRecorder);
        SetEnableGravity(bEnableGravityRecorder);
    }
    else
    {
        SetSimulatePhysics(false);
        SetEnableGravity(false);
        if (bSimulatePhysicsRecorder)
            AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
    }
}

UPrimitiveComponent *UVFDynamicMeshComponent::GetSourceComponent()
{
    return SourceComponent;
}

UVFDynamicMeshComponent *UVFDynamicMeshComponent::GetSourceVFDMComp()
{
    if (!SourceComponent)
        return nullptr;
    return Cast<UVFDynamicMeshComponent>(SourceComponent);
}