// Copyright 2026, StrangeDS. All Rights Reserved.

#include "VFGSNone.h"

#include "VFLog.h"
#include "Engine/StaticMesh.h"

#define LOCTEXT_NAMESPACE "UVFGSNone"

UDynamicMesh *UVFGSNone::
    SetDynamicMeshCollisionFromMesh_Implementation(
        UDynamicMesh *FromDynamicMesh,
        UDynamicMeshComponent *ToDynamicMeshComponent,
        FVF_GeometryScriptCollisionFromMeshOptions Options)
{
    check(FromDynamicMesh);
    check(ToDynamicMeshComponent);

    VF_LOG(Warning, TEXT("VFGSNone is used."));
    return FromDynamicMesh;
}

UDynamicMesh *UVFGSNone::
    CopyMeshFromStaticMesh_Implementation(
        UStaticMesh *FromStaticMeshAsset,
        UDynamicMesh *ToDynamicMesh,
        FVF_GeometryScriptCopyMeshFromAssetOptions AssetOptions,
        FVF_GeometryScriptMeshReadLOD RequestedLOD)
{
    check(FromStaticMeshAsset);
    check(ToDynamicMesh);
    ensureMsgf(FromStaticMeshAsset->bAllowCPUAccess,
               TEXT("Mesh %s bAllowCPUAccess needs to be true."),
               *FromStaticMeshAsset->GetName());

    VF_LOG(Warning, TEXT("VFGSNone is used."));
    return ToDynamicMesh;
}

UDynamicMesh *UVFGSNone::
    SetVertexPosition_Implementation(
        UDynamicMesh *TargetMesh,
        int VertexID,
        FVector NewPosition,
        bool &bIsValidVertex,
        bool bDeferChangeNotifications)
{
    check(TargetMesh);

    VF_LOG(Warning, TEXT("VFGSNone is used."));
    return TargetMesh;
}

UDynamicMesh *UVFGSNone::
    ApplyMeshBoolean_Implementation(
        UDynamicMesh *TargetMesh,
        FTransform TargetTransform,
        UDynamicMesh *ToolMesh,
        FTransform ToolTransform,
        EVF_GeometryScriptBooleanOperation Operation,
        FVF_GeometryScriptMeshBooleanOptions Options)
{
    check(TargetMesh);
    check(ToolMesh);

    VF_LOG(Warning, TEXT("VFGSNone is used."));
    return TargetMesh;
}

UDynamicMesh *UVFGSNone::
    ApplyMeshSelfUnion_Implementation(
        UDynamicMesh *TargetMesh,
        FVF_GeometryScriptMeshSelfUnionOptions Options)
{
    check(TargetMesh);

    VF_LOG(Warning, TEXT("VFGSNone is used."));
    return TargetMesh;
}

UDynamicMesh *UVFGSNone::
    AppendFrustum_Implementation(
        UDynamicMesh *TargetMesh,
        FVF_GeometryScriptPrimitiveOptions PrimitiveOptions,
        float ViewAngle,
        float AspectRatio,
        float StartDis,
        float EndDis)
{
    check(TargetMesh);

    VF_LOG(Warning, TEXT("VFGSNone is used."));
    return TargetMesh;
}

UDynamicMesh *UVFGSNone::
    CopyMeshFromComponent_Implementation(
        UPrimitiveComponent *Component,
        UDynamicMesh *ToDynamicMesh,
        FVF_GeometryScriptCopyMeshFromComponentOptions Options,
        bool bTransformToWorld)
{
    check(Component);
    check(ToDynamicMesh);

    VF_LOG(Warning, TEXT("VFGSNone is used."));
    return ToDynamicMesh;
}

#undef LOCTEXT_NAMESPACE