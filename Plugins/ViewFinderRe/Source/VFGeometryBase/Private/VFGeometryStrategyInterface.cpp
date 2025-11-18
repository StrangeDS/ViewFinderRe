// Copyright StrangeDS. All Rights Reserved.

#include "VFGeometryStrategyInterface.h"

UDynamicMesh *IVFGeometryStrategyInterface::
    SetDynamicMeshCollisionFromMesh_Implementation(
        UDynamicMesh *FromDynamicMesh,
        UDynamicMeshComponent *ToDynamicMeshComponent,
        FVF_GeometryScriptCollisionFromMeshOptions Options)
{
    unimplemented();
    return FromDynamicMesh;
}

UDynamicMesh *IVFGeometryStrategyInterface::
    CopyMeshFromStaticMesh_Implementation(
        UStaticMesh *FromStaticMeshAsset,
        UDynamicMesh *ToDynamicMesh,
        FVF_GeometryScriptCopyMeshFromAssetOptions AssetOptions,
        FVF_GeometryScriptMeshReadLOD RequestedLOD)
{
    unimplemented();
    return ToDynamicMesh;
}

UDynamicMesh *IVFGeometryStrategyInterface::
    SetVertexPosition_Implementation(
        UDynamicMesh *TargetMesh,
        int VertexID,
        FVector NewPosition,
        bool &bIsValidVertex,
        bool bDeferChangeNotifications)
{
    unimplemented();
    return TargetMesh;
}

UDynamicMesh *IVFGeometryStrategyInterface::
    ApplyMeshBoolean_Implementation(
        UDynamicMesh *TargetMesh,
        FTransform TargetTransform,
        UDynamicMesh *ToolMesh,
        FTransform ToolTransform,
        EVF_GeometryScriptBooleanOperation Operation,
        FVF_GeometryScriptMeshBooleanOptions Options)
{
    unimplemented();
    return TargetMesh;
}

UDynamicMesh *IVFGeometryStrategyInterface::
    ApplyMeshSelfUnion_Implementation(
        UDynamicMesh *TargetMesh,
        FVF_GeometryScriptMeshSelfUnionOptions Options)
{
    unimplemented();
    return TargetMesh;
}

UDynamicMesh *IVFGeometryStrategyInterface::
    AppendFrustum_Implementation(
        UDynamicMesh *TargetMesh,
        FVF_GeometryScriptPrimitiveOptions PrimitiveOptions,
        float Angle,
        float AspectRatio,
        float StartDis,
        float EndDis)
{
    unimplemented();
    return TargetMesh;
}

UDynamicMesh *IVFGeometryStrategyInterface::
    CopyMeshFromComponent_Implementation(
        UPrimitiveComponent *Component,
        UDynamicMesh *ToDynamicMesh,
        FVF_GeometryScriptCopyMeshFromComponentOptions Options,
        bool bTransformToWorld)
{
    unimplemented();
    return ToDynamicMesh;
}