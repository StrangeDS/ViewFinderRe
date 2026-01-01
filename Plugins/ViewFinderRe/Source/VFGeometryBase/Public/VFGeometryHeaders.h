// Copyright 2026, StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VFGeometryHeaders.generated.h"

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
	MinVolumeShapes = 6,
	LevelSets = 7,
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
	SmallestVolume = 4,
};

USTRUCT(BlueprintType)
struct VFGEOMETRYBASE_API FVF_GeometryScriptCollisionFromMeshOptions
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
	int ConvexHullTargetFaceCount = 12;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	int MaxConvexHullsPerMesh = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	float ConvexDecompositionSearchFactor = .5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	float ConvexDecompositionErrorTolerance = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	float ConvexDecompositionMinPartThickness = 1.0;

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
	RenderData,
};

USTRUCT(BlueprintType)
struct VFGEOMETRYBASE_API FVF_GeometryScriptMeshReadLOD
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
struct VFGEOMETRYBASE_API FVF_GeometryScriptCopyMeshFromAssetOptions
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bApplyBuildSettings = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bRequestTangents = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bIgnoreRemoveDegenerates = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bUseBuildScale = true;
};

// from GeometryScript/MeshBooleanFunctions.h
UENUM(BlueprintType)
enum class EVF_GeometryScriptBooleanOperation : uint8
{
	Union,
	Intersection,
	Subtract,
	TrimInside,
	TrimOutside,
	NewPolyGroupInside UMETA(DisplayName = "New PolyGroup Inside"),
	NewPolyGroupOutside UMETA(DisplayName = "New PolyGroup Outside"),
};

// Options for the output coordinate space for the mesh boolean result
UENUM(BlueprintType)
enum class E_VFGeometryScriptBooleanOutputSpace : uint8
{
	// Transform the boolean result into the local space of the target mesh
	TargetTransformSpace,
	// Transform the boolean result into the local space of the tool mesh
	ToolTransformSpace,
	// Keep the boolean result in the shared space where the boolean was computed
	SharedTransformSpace,
};

USTRUCT(BlueprintType)
struct VFGEOMETRYBASE_API FVF_GeometryScriptMeshBooleanOptions
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bFillHoles = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bSimplifyOutput = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	float SimplifyPlanarTolerance = 0.01f;

	// Whether to allow the Mesh Boolean operation to generate an empty mesh as its result
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bAllowEmptyResult = false;

	// The coordinate space to use for the result mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	E_VFGeometryScriptBooleanOutputSpace OutputTransformSpace = E_VFGeometryScriptBooleanOutputSpace::TargetTransformSpace;
};

USTRUCT(BlueprintType)
struct VFGEOMETRYBASE_API FVF_GeometryScriptMeshSelfUnionOptions
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
	PerQuad = 2,
};

UENUM(BlueprintType)
enum class EVF_GeometryScriptPrimitiveUVMode : uint8
{
	Uniform = 0,
	ScaleToFill = 1,
};

USTRUCT(BlueprintType)
struct VFGEOMETRYBASE_API FVF_GeometryScriptPrimitiveOptions
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options, meta = (DisplayName = "PolyGroup Mode"))
	EVF_GeometryScriptPrimitivePolygroupMode PolygroupMode = EVF_GeometryScriptPrimitivePolygroupMode::PerFace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bFlipOrientation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	EVF_GeometryScriptPrimitiveUVMode UVMode = EVF_GeometryScriptPrimitiveUVMode::Uniform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	int32 MaterialID = 0;
};

// from GeometryScript/SceneUtilityFunctions.h
USTRUCT(BlueprintType)
struct VFGEOMETRYBASE_API FVF_GeometryScriptCopyMeshFromComponentOptions
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bWantNormals = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bWantTangents = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bWantInstanceColors = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	FVF_GeometryScriptMeshReadLOD RequestedLOD = FVF_GeometryScriptMeshReadLOD();
};