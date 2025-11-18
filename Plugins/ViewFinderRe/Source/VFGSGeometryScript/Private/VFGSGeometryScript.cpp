// Copyright StrangeDS. All Rights Reserved.

#include "VFGSGeometryScript.h"

#include "GeometryScript/CollisionFunctions.h"
#include "GeometryScript/MeshAssetFunctions.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshBooleanFunctions.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "GeometryScript/SceneUtilityFunctions.h"

#include "VFLog.h"

// Intermediate layer parameters need to be converted to the parameter types used in the plugin
// Consider using addresses, memcpy(&b, &a, sizeof(A)), but what if the plugin updates and adds a new value?
// Manual conversion is much more robust, but also retain macro toggles
#define MANUAL_CONVERT 1

#ifndef MANUAL_CONVERT
#else
static EGeometryScriptCollisionGenerationMethod Convert(
    const EVF_GeometryScriptCollisionGenerationMethod &Meth)
{
    EGeometryScriptCollisionGenerationMethod Method;
    switch (Meth)
    {
    case EVF_GeometryScriptCollisionGenerationMethod::AlignedBoxes:
        Method = EGeometryScriptCollisionGenerationMethod::AlignedBoxes;
        break;
    case EVF_GeometryScriptCollisionGenerationMethod::OrientedBoxes:
        Method = EGeometryScriptCollisionGenerationMethod::OrientedBoxes;
        break;
    case EVF_GeometryScriptCollisionGenerationMethod::MinimalSpheres:
        Method = EGeometryScriptCollisionGenerationMethod::MinimalSpheres;
        break;
    case EVF_GeometryScriptCollisionGenerationMethod::Capsules:
        Method = EGeometryScriptCollisionGenerationMethod::Capsules;
        break;
    case EVF_GeometryScriptCollisionGenerationMethod::ConvexHulls:
        Method = EGeometryScriptCollisionGenerationMethod::ConvexHulls;
        break;
    case EVF_GeometryScriptCollisionGenerationMethod::SweptHulls:
        Method = EGeometryScriptCollisionGenerationMethod::SweptHulls;
        break;
    case EVF_GeometryScriptCollisionGenerationMethod::MinVolumeShapes:
        Method = EGeometryScriptCollisionGenerationMethod::MinVolumeShapes;
        break;
    default:
        Method = EGeometryScriptCollisionGenerationMethod::ConvexHulls;
        break;
    }
    return Method;
}

static EGeometryScriptSweptHullAxis Convert(
    const EVF_GeometryScriptSweptHullAxis &Ax)
{
    EGeometryScriptSweptHullAxis Axis;
    switch (Ax)
    {
    case EVF_GeometryScriptSweptHullAxis::X:
        Axis = EGeometryScriptSweptHullAxis::X;
        break;
    case EVF_GeometryScriptSweptHullAxis::Y:
        Axis = EGeometryScriptSweptHullAxis::Y;
        break;
    case EVF_GeometryScriptSweptHullAxis::Z:
        Axis = EGeometryScriptSweptHullAxis::Z;
        break;
    case EVF_GeometryScriptSweptHullAxis::SmallestBoxDimension:
        Axis = EGeometryScriptSweptHullAxis::SmallestBoxDimension;
        break;
    case EVF_GeometryScriptSweptHullAxis::SmallestVolume:
        Axis = EGeometryScriptSweptHullAxis::SmallestVolume;
        break;
    default:
        Axis = EGeometryScriptSweptHullAxis::Z;
        break;
    }
    return Axis;
}

static FGeometryScriptCollisionFromMeshOptions Convert(
    const FVF_GeometryScriptCollisionFromMeshOptions &Options_)
{
    FGeometryScriptCollisionFromMeshOptions Options{
        Options_.bEmitTransaction,
        Convert(Options_.Method),
        Options_.bAutoDetectSpheres,
        Options_.bAutoDetectBoxes,
        Options_.bAutoDetectCapsules,
        Options_.MinThickness,
        Options_.bSimplifyHulls,
        Options_.ConvexHullTargetFaceCount,
        Options_.MaxConvexHullsPerMesh,
        Options_.ConvexDecompositionSearchFactor,
        Options_.ConvexDecompositionErrorTolerance,
        Options_.ConvexDecompositionMinPartThickness,
        Options_.SweptHullSimplifyTolerance,
        Convert(Options_.SweptHullAxis),
        Options_.bRemoveFullyContainedShapes,
        Options_.MaxShapeCount};
    return Options;
}

static EGeometryScriptBooleanOperation Convert(
    const EVF_GeometryScriptBooleanOperation &Operation_)
{
    EGeometryScriptBooleanOperation Operation;
    switch (Operation_)
    {
    case EVF_GeometryScriptBooleanOperation::Union:
        Operation = EGeometryScriptBooleanOperation::Union;
        break;
    case EVF_GeometryScriptBooleanOperation::Intersection:
        Operation = EGeometryScriptBooleanOperation::Intersection;
        break;
    case EVF_GeometryScriptBooleanOperation::Subtract:
        Operation = EGeometryScriptBooleanOperation::Subtract;
        break;
    default:
        Operation = EGeometryScriptBooleanOperation::Subtract;
        break;
    }
    return Operation;
}

static FGeometryScriptMeshBooleanOptions Convert(
    const FVF_GeometryScriptMeshBooleanOptions &Options_)
{
    FGeometryScriptMeshBooleanOptions Options{
        Options_.bFillHoles,
        Options_.bSimplifyOutput,
        Options_.SimplifyPlanarTolerance};
    return Options;
}

static FGeometryScriptCopyMeshFromAssetOptions Convert(
    const FVF_GeometryScriptCopyMeshFromAssetOptions &Options_)
{
    FGeometryScriptCopyMeshFromAssetOptions Options{
        Options_.bApplyBuildSettings,
        Options_.bRequestTangents,
        Options_.bIgnoreRemoveDegenerates};
    return Options;
};

static EGeometryScriptLODType Convert(
    const EVF_GeometryScriptLODType &Type_)
{
    EGeometryScriptLODType Type;
    switch (Type_)
    {
    case EVF_GeometryScriptLODType::MaxAvailable:
        Type = EGeometryScriptLODType::MaxAvailable;
        break;
    case EVF_GeometryScriptLODType::HiResSourceModel:
        Type = EGeometryScriptLODType::HiResSourceModel;
        break;
    case EVF_GeometryScriptLODType::SourceModel:
        Type = EGeometryScriptLODType::SourceModel;
        break;
    case EVF_GeometryScriptLODType::RenderData:
        Type = EGeometryScriptLODType::RenderData;
        break;
    default:
        Type = EGeometryScriptLODType::RenderData;
        break;
    }
    return Type;
}

static FGeometryScriptMeshReadLOD Convert(
    const FVF_GeometryScriptMeshReadLOD &LOD_)
{
    FGeometryScriptMeshReadLOD LOD{
        Convert(LOD_.LODType),
        LOD_.LODIndex};
    return LOD;
};

FGeometryScriptMeshSelfUnionOptions Convert(
    const FVF_GeometryScriptMeshSelfUnionOptions &Options_)
{
    FGeometryScriptMeshSelfUnionOptions Options{
        Options_.bFillHoles,
        Options_.bTrimFlaps,
        Options_.bSimplifyOutput,
        Options_.SimplifyPlanarTolerance,
        Options_.WindingThreshold};
    return Options;
}

EGeometryScriptPrimitivePolygroupMode Convert(
    const EVF_GeometryScriptPrimitivePolygroupMode &Mode_)
{
    EGeometryScriptPrimitivePolygroupMode Mode;
    switch (Mode_)
    {
    case EVF_GeometryScriptPrimitivePolygroupMode::SingleGroup:
        Mode = EGeometryScriptPrimitivePolygroupMode::SingleGroup;
        break;
    case EVF_GeometryScriptPrimitivePolygroupMode::PerFace:
        Mode = EGeometryScriptPrimitivePolygroupMode::PerFace;
        break;
    case EVF_GeometryScriptPrimitivePolygroupMode::PerQuad:
        Mode = EGeometryScriptPrimitivePolygroupMode::PerQuad;
        break;
    default:
        Mode = EGeometryScriptPrimitivePolygroupMode::PerFace;
        break;
    }
    return Mode;
}

EGeometryScriptPrimitiveUVMode Convert(
    const EVF_GeometryScriptPrimitiveUVMode &Mode_)
{
    EGeometryScriptPrimitiveUVMode Mode;
    switch (Mode_)
    {
    case EVF_GeometryScriptPrimitiveUVMode::Uniform:
        Mode = EGeometryScriptPrimitiveUVMode::Uniform;
        break;
    case EVF_GeometryScriptPrimitiveUVMode::ScaleToFill:
        Mode = EGeometryScriptPrimitiveUVMode::ScaleToFill;
        break;
    default:
        Mode = EGeometryScriptPrimitiveUVMode::Uniform;
        break;
    }
    return Mode;
}

FGeometryScriptPrimitiveOptions Convert(
    const FVF_GeometryScriptPrimitiveOptions &Options_)
{
    FGeometryScriptPrimitiveOptions Options{
        Convert(Options_.PolygroupMode),
        Options_.bFlipOrientation,
        Convert(Options_.UVMode)};
    return Options;
}

FGeometryScriptCopyMeshFromComponentOptions Convert(
    const FVF_GeometryScriptCopyMeshFromComponentOptions &Options_)
{
    FGeometryScriptCopyMeshFromComponentOptions Options{
        Options_.bWantNormals,
        Options_.bWantTangents,
        Convert(Options_.RequestedLOD)};
    return Options;
}
#endif // Macro MANUAL_CONVERT ends.

#include "DynamicSubmesh3.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "Physics/PhysicsDataCollection.h"
#include "Selections/MeshConnectedComponents.h"
#include "ShapeApproximation/MeshSimpleShapeApproximation.h"
#include "Async/ParallelFor.h"

using namespace UE::Geometry;

// from GeometryScript/CollisionFunctions.cpp UELocal::ComputeCollisionFromMesh()
static void ComputeCollisionFromMesh(
    const FDynamicMesh3 &Mesh,
    FKAggregateGeom &GeneratedCollision,
    FVF_GeometryScriptCollisionFromMeshOptions &Options)
{
    FPhysicsDataCollection NewCollision;

    FMeshConnectedComponents Components(&Mesh);
    Components.FindConnectedTriangles();
    int32 NumComponents = Components.Num();

    TArray<FDynamicMesh3> Submeshes;
    TArray<const FDynamicMesh3 *> SubmeshPointers;

    if (NumComponents == 1)
    {
        SubmeshPointers.Add(&Mesh);
    }
    else
    {
        Submeshes.SetNum(NumComponents);
        SubmeshPointers.SetNum(NumComponents);
        ParallelFor(NumComponents, [&](int32 k)
                    {
			FDynamicSubmesh3 Submesh(&Mesh, Components[k].Indices, (int32)EMeshComponents::None, false);
			Submeshes[k] = MoveTemp(Submesh.GetSubmesh());
			SubmeshPointers[k] = &Submeshes[k]; });
    }

    FMeshSimpleShapeApproximation ShapeGenerator;
    ShapeGenerator.InitializeSourceMeshes(SubmeshPointers);

    ShapeGenerator.bDetectSpheres = Options.bAutoDetectSpheres;
    ShapeGenerator.bDetectBoxes = Options.bAutoDetectBoxes;
    ShapeGenerator.bDetectCapsules = Options.bAutoDetectCapsules;

    ShapeGenerator.MinDimension = Options.MinThickness;

    switch (Options.Method)
    {
    case EVF_GeometryScriptCollisionGenerationMethod::AlignedBoxes:
        ShapeGenerator.Generate_AlignedBoxes(NewCollision.Geometry);
        break;
    case EVF_GeometryScriptCollisionGenerationMethod::OrientedBoxes:
        ShapeGenerator.Generate_OrientedBoxes(NewCollision.Geometry);
        break;
    case EVF_GeometryScriptCollisionGenerationMethod::MinimalSpheres:
        ShapeGenerator.Generate_MinimalSpheres(NewCollision.Geometry);
        break;
    case EVF_GeometryScriptCollisionGenerationMethod::Capsules:
        ShapeGenerator.Generate_Capsules(NewCollision.Geometry);
        break;
    case EVF_GeometryScriptCollisionGenerationMethod::ConvexHulls:
        ShapeGenerator.bSimplifyHulls = Options.bSimplifyHulls;
        ShapeGenerator.HullTargetFaceCount = Options.ConvexHullTargetFaceCount;
        if (Options.MaxConvexHullsPerMesh > 1)
        {
            ShapeGenerator.ConvexDecompositionMaxPieces = Options.MaxConvexHullsPerMesh;
            ShapeGenerator.ConvexDecompositionSearchFactor = Options.ConvexDecompositionSearchFactor;
            ShapeGenerator.ConvexDecompositionErrorTolerance = Options.ConvexDecompositionErrorTolerance;
            ShapeGenerator.ConvexDecompositionMinPartThickness = Options.ConvexDecompositionMinPartThickness;
            ShapeGenerator.Generate_ConvexHullDecompositions(NewCollision.Geometry);
        }
        else
        {
            ShapeGenerator.Generate_ConvexHulls(NewCollision.Geometry);
        }
        break;
    case EVF_GeometryScriptCollisionGenerationMethod::SweptHulls:
        ShapeGenerator.bSimplifyHulls = Options.bSimplifyHulls;
        ShapeGenerator.HullSimplifyTolerance = Options.SweptHullSimplifyTolerance;
        ShapeGenerator.Generate_ProjectedHulls(NewCollision.Geometry,
                                               static_cast<FMeshSimpleShapeApproximation::EProjectedHullAxisMode>(Options.SweptHullAxis));
        break;
    case EVF_GeometryScriptCollisionGenerationMethod::MinVolumeShapes:
        ShapeGenerator.Generate_MinVolume(NewCollision.Geometry);
        break;
    }

    if (Options.bRemoveFullyContainedShapes && Components.Num() > 1)
    {
        NewCollision.Geometry.RemoveContainedGeometry();
    }

    if (Options.MaxShapeCount > 0 && Options.MaxShapeCount < Components.Num())
    {
        NewCollision.Geometry.FilterByVolume(Options.MaxShapeCount);
    }

    NewCollision.CopyGeometryToAggregate();
    GeneratedCollision = NewCollision.AggGeom;
}

UDynamicMesh *UVFGSGeometryScript::
    SetDynamicMeshCollisionFromMesh_Implementation(
        UDynamicMesh *FromDynamicMesh,
        UDynamicMeshComponent *ToDynamicMeshComponent,
        FVF_GeometryScriptCollisionFromMeshOptions Options)
{
    check(FromDynamicMesh);
    check(ToDynamicMeshComponent);

    return UGeometryScriptLibrary_CollisionFunctions::SetDynamicMeshCollisionFromMesh(
        FromDynamicMesh,
        ToDynamicMeshComponent,
        Convert(Options),
        nullptr);
}

UDynamicMesh *UVFGSGeometryScript::
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

    EGeometryScriptOutcomePins Outcome;
    return UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshFromStaticMesh(
        FromStaticMeshAsset,
        ToDynamicMesh,
        Convert(AssetOptions),
        Convert(RequestedLOD),
        Outcome,
        nullptr);
}

UDynamicMesh *UVFGSGeometryScript::
    SetVertexPosition_Implementation(
        UDynamicMesh *TargetMesh,
        int VertexID,
        FVector NewPosition,
        bool &bIsValidVertex,
        bool bDeferChangeNotifications)
{
    check(TargetMesh);

    return UGeometryScriptLibrary_MeshBasicEditFunctions::SetVertexPosition(
        TargetMesh,
        VertexID,
        NewPosition,
        bIsValidVertex,
        bDeferChangeNotifications);
}

UDynamicMesh *UVFGSGeometryScript::
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

    return UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshBoolean(
        TargetMesh,
        TargetTransform,
        ToolMesh,
        ToolTransform,
        Convert(Operation),
        Convert(Options),
        nullptr);
}

UDynamicMesh *UVFGSGeometryScript::
    ApplyMeshSelfUnion_Implementation(
        UDynamicMesh *TargetMesh,
        FVF_GeometryScriptMeshSelfUnionOptions Options)
{
    check(TargetMesh);

    return UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshSelfUnion(
        TargetMesh,
        Convert(Options),
        nullptr);
}

UDynamicMesh *UVFGSGeometryScript::
    AppendFrustum_Implementation(
        UDynamicMesh *TargetMesh,
        FVF_GeometryScriptPrimitiveOptions PrimitiveOptions,
        float Angle,
        float AspectRatio,
        float StartDis,
        float EndDis)
{
    check(TargetMesh);
    // Implementation: Modify vertex positions on a simple box to create a frustum.
    // A better approach would be to write an FMeshShapeGenerator.

    float DimensionX = 100.0f, DimensionY = 100.0f, DimensionZ = 100.0f;
    int32 StepsX = 0, StepsY = 0, StepsZ = 0;

    UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendBox(
        TargetMesh,
        Convert(PrimitiveOptions),
        FTransform::Identity,
        DimensionX,
        DimensionY,
        DimensionZ,
        StepsX,
        StepsY,
        StepsZ,
        EGeometryScriptPrimitiveOriginMode::Center,
        nullptr);

    // Calculate position of each vertex in frustum.
    TArray<FVector> Positions;
    {
        Positions.Reserve(8);
        auto CalculatePositions = [&](float Distance)
        {
            float x = Distance;
            float y = Distance * tanf(FMath::DegreesToRadians(Angle) / 2);
            float z = y / AspectRatio;
            Positions.Push({x, -y, -z});
            Positions.Push({x, y, -z});
            Positions.Push({x, y, z});
            Positions.Push({x, -y, z});
        };
        CalculatePositions(StartDis);
        CalculatePositions(EndDis);
    }
    for (int i = 0; i < 8; i++)
    {
        bool Success;
        IVFGeometryStrategyInterface::Execute_SetVertexPosition(
            this, TargetMesh, i, Positions[i], Success, i != 7);
    }

    return TargetMesh;
}

#include "Components/PrimitiveComponent.h"

UDynamicMesh *UVFGSGeometryScript::
    CopyMeshFromComponent_Implementation(
        UPrimitiveComponent *Component,
        UDynamicMesh *ToDynamicMesh,
        FVF_GeometryScriptCopyMeshFromComponentOptions Options,
        bool bTransformToWorld)
{
    check(Component);
    check(ToDynamicMesh);

    EGeometryScriptOutcomePins Outcome;
    FTransform LocalToWorld = FTransform::Identity;
    return UGeometryScriptLibrary_SceneUtilityFunctions::CopyMeshFromComponent(
        Component,
        ToDynamicMesh,
        Convert(Options),
        bTransformToWorld,
        LocalToWorld,
        Outcome,
        nullptr);
}