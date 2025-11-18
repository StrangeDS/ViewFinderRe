// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VFGeometryStrategyInterface.h"
#include "VFGSNone.generated.h"

// Do nothing.
UCLASS(BlueprintType, Blueprintable, ClassGroup = (ViewFinder))
class VFGEOMETRYBASE_API UVFGSNone
	: public UObject,
	  public IVFGeometryStrategyInterface
{
	GENERATED_BODY()

	virtual UDynamicMesh *SetDynamicMeshCollisionFromMesh_Implementation(
		UDynamicMesh *FromDynamicMesh,
		UDynamicMeshComponent *ToDynamicMeshComponent,
		FVF_GeometryScriptCollisionFromMeshOptions Options) override;

	virtual UDynamicMesh *CopyMeshFromStaticMesh_Implementation(
		UStaticMesh *FromStaticMeshAsset,
		UDynamicMesh *ToDynamicMesh,
		FVF_GeometryScriptCopyMeshFromAssetOptions AssetOptions,
		FVF_GeometryScriptMeshReadLOD RequestedLOD);

	virtual UDynamicMesh *SetVertexPosition_Implementation(
		UDynamicMesh *TargetMesh,
		int VertexID,
		FVector NewPosition,
		bool &bIsValidVertex,
		bool bDeferChangeNotifications = false);

	virtual UDynamicMesh *ApplyMeshBoolean_Implementation(
		UDynamicMesh *TargetMesh,
		FTransform TargetTransform,
		UDynamicMesh *ToolMesh,
		FTransform ToolTransform,
		EVF_GeometryScriptBooleanOperation Operation,
		FVF_GeometryScriptMeshBooleanOptions Options);

	virtual UDynamicMesh *ApplyMeshSelfUnion_Implementation(
		UDynamicMesh *TargetMesh,
		FVF_GeometryScriptMeshSelfUnionOptions Options);

	virtual UDynamicMesh *AppendFrustum_Implementation(
		UDynamicMesh *TargetMesh,
		FVF_GeometryScriptPrimitiveOptions PrimitiveOptions,
		float Angle = 90.0f,
		float AspectRatio = 1.6667f,
		float StartDis = 10.0f,
		float EndDis = 10000.0f);

	virtual UDynamicMesh *CopyMeshFromComponent_Implementation(
		UPrimitiveComponent *Component,
		UDynamicMesh *ToDynamicMesh,
		FVF_GeometryScriptCopyMeshFromComponentOptions Options,
		bool bTransformToWorld = true);
};