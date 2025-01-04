#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VFGeometryFunctions.generated.h"

// 此处基本都是抄的GeometryScripting的代码.
// GeometryScripting中有些函数不是完全符合需求, 无奈只能抄一份.
// 干脆不依赖GeometryScripting.

// from GeometryScript/CollisionFunctions.h
UENUM(BlueprintType)
enum class EVF_GeometryScriptCollisionGenerationMethod : uint8
{
	AlignedBoxes = 0,
	OrientedBoxes = 1,
	MinimalSpheres = 2,
	Capsules = 3,
	ConvexHulls = 4,
	SweptHulls = 5,
	MinVolumeShapes = 6
};

UENUM(BlueprintType)
enum class EVF_GeometryScriptSweptHullAxis : uint8
{
	X = 0,
	Y = 1,
	Z = 2,
	/** Use X/Y/Z axis with smallest axis-aligned-bounding-box dimension */
	SmallestBoxDimension = 3,
	/** Compute projected hull for each of X/Y/Z axes and use the one that has the smallest volume  */
	SmallestVolume = 4
};

USTRUCT(BlueprintType)
struct VIEWFINDERCORE_API FVF_GeometryScriptCollisionFromMeshOptions
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bEmitTransaction = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	EVF_GeometryScriptCollisionGenerationMethod Method = EVF_GeometryScriptCollisionGenerationMethod::ConvexHulls;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bAutoDetectSpheres = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bAutoDetectBoxes = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bAutoDetectCapsules = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	float MinThickness = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bSimplifyHulls = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	int ConvexHullTargetFaceCount = 25;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	int MaxConvexHullsPerMesh = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	float ConvexDecompositionSearchFactor = .5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	float ConvexDecompositionErrorTolerance = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	float ConvexDecompositionMinPartThickness = 0.1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	float SweptHullSimplifyTolerance = 0.1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	EVF_GeometryScriptSweptHullAxis SweptHullAxis = EVF_GeometryScriptSweptHullAxis::Z;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bRemoveFullyContainedShapes = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	int MaxShapeCount = 0;
};

// from GeometryScript/GeometryScriptTypes.h
UENUM(BlueprintType)
enum class EVF_GeometryScriptLODType : uint8
{
	MaxAvailable,
	HiResSourceModel,
	SourceModel,
	RenderData
};

USTRUCT(BlueprintType)
struct VIEWFINDERCORE_API FVF_GeometryScriptMeshReadLOD
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, Category = LOD)
	EVF_GeometryScriptLODType LODType = EVF_GeometryScriptLODType::MaxAvailable;

	UPROPERTY(BlueprintReadWrite, Category = LOD)
	int32 LODIndex = 0;
};

// from GeometryScript/MeshAssetFunctions.h
USTRUCT(BlueprintType)
struct VIEWFINDERCORE_API FVF_GeometryScriptCopyMeshFromAssetOptions
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bApplyBuildSettings = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bRequestTangents = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bIgnoreRemoveDegenerates = true;
};

// from GeometryScript/MeshBooleanFunctions.h
UENUM(BlueprintType)
enum class EVF_GeometryScriptBooleanOperation : uint8
{
	Union,
	Intersection,
	Subtract
};

USTRUCT(BlueprintType)
struct VIEWFINDERCORE_API FVF_GeometryScriptMeshBooleanOptions
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bFillHoles = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bSimplifyOutput = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	float SimplifyPlanarTolerance = 0.01f;
};

USTRUCT(BlueprintType)
struct VIEWFINDERCORE_API FVF_GeometryScriptMeshSelfUnionOptions
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bFillHoles = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bTrimFlaps = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bSimplifyOutput = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	float SimplifyPlanarTolerance = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	float WindingThreshold = 0.5f;
};

// from GeometryScript/MeshPrimitiveFunctions.h
UENUM(BlueprintType)
enum class EVF_GeometryScriptPrimitivePolygroupMode : uint8
{
	SingleGroup = 0,
	PerFace = 1,
	PerQuad = 2
};

UENUM(BlueprintType)
enum class EVF_GeometryScriptPrimitiveUVMode : uint8
{
	Uniform = 0,
	ScaleToFill = 1
};

USTRUCT(BlueprintType)
struct VIEWFINDERCORE_API FVF_GeometryScriptPrimitiveOptions
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options, meta = (DisplayName = "PolyGroup Mode"))
	EVF_GeometryScriptPrimitivePolygroupMode PolygroupMode = EVF_GeometryScriptPrimitivePolygroupMode::PerFace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bFlipOrientation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	EVF_GeometryScriptPrimitiveUVMode UVMode = EVF_GeometryScriptPrimitiveUVMode::Uniform;
};

// from GeometryScript/SceneUtilityFunctions.h
USTRUCT(BlueprintType)
struct VIEWFINDERCORE_API FVF_GeometryScriptCopyMeshFromComponentOptions
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bWantNormals = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bWantTangents = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	FVF_GeometryScriptMeshReadLOD RequestedLOD = FVF_GeometryScriptMeshReadLOD();
};

UCLASS(meta = (ScriptName = "VFGeometryFunctions"))
class VIEWFINDERCORE_API UVFGeometryFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// from GeometryScript/CollisionFunctions.h
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static UPARAM(DisplayName = "Dynamic Mesh") UDynamicMesh *SetDynamicMeshCollisionFromMesh(
		UDynamicMesh *FromDynamicMesh,
		UDynamicMeshComponent *ToDynamicMeshComponent,
		FVF_GeometryScriptCollisionFromMeshOptions Options);

	// from GeometryScript/MeshAssetFunctions.h
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static UPARAM(DisplayName = "Dynamic Mesh") UDynamicMesh *CopyMeshFromStaticMesh(
		UStaticMesh *FromStaticMeshAsset,
		UDynamicMesh *ToDynamicMesh,
		FVF_GeometryScriptCopyMeshFromAssetOptions AssetOptions,
		FVF_GeometryScriptMeshReadLOD RequestedLOD);

	// from GeometryScript/MeshBasicEditFunctions.h
	UFUNCTION(BlueprintCallable, Category = "ViewFinder", meta = (ScriptMethod))
	static UPARAM(DisplayName = "Target Mesh") UDynamicMesh *SetVertexPosition(
		UDynamicMesh *TargetMesh,
		int VertexID,
		FVector NewPosition,
		bool &bIsValidVertex,
		bool bDeferChangeNotifications = false);

	// from GeometryScript/MeshBooleanFunctions.h
	UFUNCTION(BlueprintCallable, Category = "ViewFinder", meta = (ScriptMethod))
	static UPARAM(DisplayName = "Target Mesh") UDynamicMesh *ApplyMeshBoolean(
		UDynamicMesh *TargetMesh,
		FTransform TargetTransform,
		UDynamicMesh *ToolMesh,
		FTransform ToolTransform,
		EVF_GeometryScriptBooleanOperation Operation,
		FVF_GeometryScriptMeshBooleanOptions Options);

	// from GeometryScript/MeshBooleanFunctions.h
	UFUNCTION(BlueprintCallable, Category = "ViewFinder", meta = (ScriptMethod))
	static UPARAM(DisplayName = "Target Mesh") UDynamicMesh *ApplyMeshSelfUnion(
		UDynamicMesh *TargetMesh,
		FVF_GeometryScriptMeshSelfUnionOptions Options);

	// from GeometryScript/MeshPrimitiveFunctions.h
	UFUNCTION(BlueprintCallable, Category = "ViewFinder", meta = (ScriptMethod))
	static UPARAM(DisplayName = "Target Mesh") UDynamicMesh *AppendFrustum(
		UDynamicMesh *TargetMesh,
		FVF_GeometryScriptPrimitiveOptions PrimitiveOptions,
		float Angle,
		float AspectRatio,
		float StartDis,
		float EndDis);

	// from GeometryScript/SceneUtilityFunctions.h
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static UPARAM(DisplayName = "Dynamic Mesh") UDynamicMesh *CopyMeshFromComponent(
		UPrimitiveComponent *Component,
		UDynamicMesh *ToDynamicMesh,
		FVF_GeometryScriptCopyMeshFromComponentOptions Options,
		bool bTransformToWorld = true);
};
