// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VFGeometryHeaders.h"
#include "VFGeometryFunctions.generated.h"

UCLASS(meta = (ScriptName = "VFGeometryFunctions"))
class VFGEOMETRY_API UVFGeometryFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static bool IsEditorCreated(UObject *Outer);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static UVFDynamicMeshComponent *GetVFDMComp(
		UObject *Outer,
		const TSubclassOf<UVFDynamicMeshComponent> &Class);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static bool ReturnVFDMComp(UObject *Outer);

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static UObject *GetGeometryStrategyCDO();

	// from GeometryScript/CollisionFunctions.h
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static UPARAM(DisplayName = "Dynamic Mesh")
		UDynamicMesh *SetDynamicMeshCollisionFromMesh(
			UDynamicMesh *FromDynamicMesh,
			UDynamicMeshComponent *ToDynamicMeshComponent,
			FVF_GeometryScriptCollisionFromMeshOptions Options);

	// from GeometryScript/MeshAssetFunctions.h
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static UPARAM(DisplayName = "Dynamic Mesh")
		UDynamicMesh *CopyMeshFromStaticMesh(
			UStaticMesh *FromStaticMeshAsset,
			UDynamicMesh *ToDynamicMesh,
			FVF_GeometryScriptCopyMeshFromAssetOptions AssetOptions,
			FVF_GeometryScriptMeshReadLOD RequestedLOD);

	// from GeometryScript/MeshBasicEditFunctions.h
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static UPARAM(DisplayName = "Target Mesh")
		UDynamicMesh *SetVertexPosition(
			UDynamicMesh *TargetMesh,
			int VertexID,
			FVector NewPosition,
			bool &bIsValidVertex,
			bool bDeferChangeNotifications = false);

	// from GeometryScript/MeshBooleanFunctions.h
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static UPARAM(DisplayName = "Target Mesh")
		UDynamicMesh *ApplyMeshBoolean(
			UDynamicMesh *TargetMesh,
			FTransform TargetTransform,
			UDynamicMesh *ToolMesh,
			FTransform ToolTransform,
			EVF_GeometryScriptBooleanOperation Operation,
			FVF_GeometryScriptMeshBooleanOptions Options);

	// from GeometryScript/MeshBooleanFunctions.h
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static UPARAM(DisplayName = "Target Mesh")
		UDynamicMesh *ApplyMeshSelfUnion(
			UDynamicMesh *TargetMesh,
			FVF_GeometryScriptMeshSelfUnionOptions Options);

	// from GeometryScript/MeshPrimitiveFunctions.h
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static UPARAM(DisplayName = "Target Mesh")
		UDynamicMesh *AppendFrustum(
			UDynamicMesh *TargetMesh,
			FVF_GeometryScriptPrimitiveOptions PrimitiveOptions,
			float Angle = 90.0f,
			float AspectRatio = 1.6667f,
			float StartDis = 10.0f,
			float EndDis = 10000.0f);

	// from GeometryScript/SceneUtilityFunctions.h
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static UPARAM(DisplayName = "Dynamic Mesh")
		UDynamicMesh *CopyMeshFromComponent(
			UPrimitiveComponent *Component,
			UDynamicMesh *ToDynamicMesh,
			FVF_GeometryScriptCopyMeshFromComponentOptions Options,
			bool bTransformToWorld = true);
};