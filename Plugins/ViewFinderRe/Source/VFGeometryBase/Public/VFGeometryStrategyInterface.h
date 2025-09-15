#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VFGeometryHeaders.h"
#include "VFGeometryStrategyInterface.generated.h"

class UDynamicMesh;
class UDynamicMeshComponent;

// 策略模式接口, 实现几何操作.
UINTERFACE(MinimalAPI)
class UVFGeometryStrategyInterface : public UInterface
{
	GENERATED_BODY()
};

// 策略模式接口, 实现几何操作.
class VFGEOMETRYBASE_API IVFGeometryStrategyInterface
{
	GENERATED_BODY()

public:
	// from GeometryScript/CollisionFunctions.h
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent,
			  Category = "ViewFinder")
	UPARAM(DisplayName = "Dynamic Mesh")
	UDynamicMesh *SetDynamicMeshCollisionFromMesh(
		UDynamicMesh *FromDynamicMesh,
		UDynamicMeshComponent *ToDynamicMeshComponent,
		FVF_GeometryScriptCollisionFromMeshOptions Options);
	virtual UDynamicMesh *SetDynamicMeshCollisionFromMesh_Implementation(
		UDynamicMesh *FromDynamicMesh,
		UDynamicMeshComponent *ToDynamicMeshComponent,
		FVF_GeometryScriptCollisionFromMeshOptions Options);

	// from GeometryScript/MeshAssetFunctions.h
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent,
			  Category = "ViewFinder")
	UPARAM(DisplayName = "Dynamic Mesh")
	UDynamicMesh *CopyMeshFromStaticMesh(
		UStaticMesh *FromStaticMeshAsset,
		UDynamicMesh *ToDynamicMesh,
		FVF_GeometryScriptCopyMeshFromAssetOptions AssetOptions,
		FVF_GeometryScriptMeshReadLOD RequestedLOD);
	virtual UDynamicMesh *CopyMeshFromStaticMesh_Implementation(
		UStaticMesh *FromStaticMeshAsset,
		UDynamicMesh *ToDynamicMesh,
		FVF_GeometryScriptCopyMeshFromAssetOptions AssetOptions,
		FVF_GeometryScriptMeshReadLOD RequestedLOD);

	// from GeometryScript/MeshBasicEditFunctions.h
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent,
			  Category = "ViewFinder")
	UPARAM(DisplayName = "Target Mesh")
	UDynamicMesh *SetVertexPosition(
		UDynamicMesh *TargetMesh,
		int VertexID,
		FVector NewPosition,
		bool &bIsValidVertex,
		bool bDeferChangeNotifications = false);
	virtual UDynamicMesh *SetVertexPosition_Implementation(
		UDynamicMesh *TargetMesh,
		int VertexID,
		FVector NewPosition,
		bool &bIsValidVertex,
		bool bDeferChangeNotifications = false);

	// from GeometryScript/MeshBooleanFunctions.h
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent,
			  Category = "ViewFinder")
	UPARAM(DisplayName = "Target Mesh")
	UDynamicMesh *ApplyMeshBoolean(
		UDynamicMesh *TargetMesh,
		FTransform TargetTransform,
		UDynamicMesh *ToolMesh,
		FTransform ToolTransform,
		EVF_GeometryScriptBooleanOperation Operation,
		FVF_GeometryScriptMeshBooleanOptions Options);
	virtual UDynamicMesh *ApplyMeshBoolean_Implementation(
		UDynamicMesh *TargetMesh,
		FTransform TargetTransform,
		UDynamicMesh *ToolMesh,
		FTransform ToolTransform,
		EVF_GeometryScriptBooleanOperation Operation,
		FVF_GeometryScriptMeshBooleanOptions Options);

	// from GeometryScript/MeshBooleanFunctions.h
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent,
			  Category = "ViewFinder")
	UPARAM(DisplayName = "Target Mesh")
	UDynamicMesh *ApplyMeshSelfUnion(
		UDynamicMesh *TargetMesh,
		FVF_GeometryScriptMeshSelfUnionOptions Options);
	virtual UDynamicMesh *ApplyMeshSelfUnion_Implementation(
		UDynamicMesh *TargetMesh,
		FVF_GeometryScriptMeshSelfUnionOptions Options);

	// from GeometryScript/MeshPrimitiveFunctions.h
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent,
			  Category = "ViewFinder")
	UPARAM(DisplayName = "Target Mesh")
	UDynamicMesh *AppendFrustum(
		UDynamicMesh *TargetMesh,
		FVF_GeometryScriptPrimitiveOptions PrimitiveOptions,
		float Angle = 90.0f,
		float AspectRatio = 1.6667f,
		float StartDis = 10.0f,
		float EndDis = 10000.0f);
	virtual UDynamicMesh *AppendFrustum_Implementation(
		UDynamicMesh *TargetMesh,
		FVF_GeometryScriptPrimitiveOptions PrimitiveOptions,
		float Angle = 90.0f,
		float AspectRatio = 1.6667f,
		float StartDis = 10.0f,
		float EndDis = 10000.0f);

	// from GeometryScript/SceneUtilityFunctions.h
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent,
			  Category = "ViewFinder")
	UPARAM(DisplayName = "Dynamic Mesh")
	UDynamicMesh *CopyMeshFromComponent(
		UPrimitiveComponent *Component,
		UDynamicMesh *ToDynamicMesh,
		FVF_GeometryScriptCopyMeshFromComponentOptions Options,
		bool bTransformToWorld = true);
	virtual UDynamicMesh *CopyMeshFromComponent_Implementation(
		UPrimitiveComponent *Component,
		UDynamicMesh *ToDynamicMesh,
		FVF_GeometryScriptCopyMeshFromComponentOptions Options,
		bool bTransformToWorld = true);
};
