#include "VFDynamicMeshComponent.h"

#include "VFCommon.h"
#include "VFDMCompPoolWorldSubsystem.h"
#include "VFGeometryFunctions.h"

UVFDynamicMeshComponent::UVFDynamicMeshComponent(const FObjectInitializer &ObjectInitializer)
    : UDynamicMeshComponent(ObjectInitializer)
{
    SetMobility(EComponentMobility::Movable);
}

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

    UpdateMaterials();

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
    UVFGeometryFunctions::SetDynamicMeshCollisionFromMesh(
        MeshObject,
        this,
        Options);
}

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

void UVFDynamicMeshComponent::RestoreSourceComponent()
{
    if (!SourceComponent)
        return;

    SourceComponent->SetHiddenInGame(false);
    SourceComponent->SetCollisionProfileName(GetCollisionProfileName());
    SourceComponent->SetCollisionEnabled(GetCollisionEnabled());
    SourceComponent->SetSimulatePhysics(bSimulatePhysicsRecorder);
    SourceComponent->BodyInstance.bSimulatePhysics = bEnableGravityRecorder;
}

#if WITH_EDITOR
void UVFDynamicMeshComponent::ReplaceMeshForComponentInEditor()
{
    const FScopedTransaction Transaction(FText::FromString("ReplaceMeshForComponentInEditor"));

    auto Source = Cast<UPrimitiveComponent>(GetAttachParent());
    if (!Source)
        VF_LOG(Error, TEXT("%s GetAttachParent() is not a UPrimitiveComponent."), __FUNCTIONW__);

    Modify();
    ReplaceMeshForComponent(Source);
    SourceComponent = nullptr;
    VF_LOG(Log, TEXT("%s has Replaced parent Component."), *GetName());
}

void UVFDynamicMeshComponent::RestoreSourceComponentInEditor()
{
    const FScopedTransaction Transaction(FText::FromString("RestoreSourceComponentInEditor"));

    auto Source = Cast<UPrimitiveComponent>(GetAttachParent());
    if (!Source)
        VF_LOG(Error, TEXT("%s GetAttachParent() is not a UPrimitiveComponent."), __FUNCTIONW__);

    Modify();
    SourceComponent = Source;
    RestoreSourceComponent();
    SourceComponent = nullptr;
    VF_LOG(Log, TEXT("%s has Restored parent Component."), *GetName());
}
#endif