#include "VFGeometryFunctions.h"

#include "DynamicMesh/DynamicMesh3.h"
#include "DynamicMesh/MeshTransforms.h"
#include "UDynamicMesh.h"

#include "Async/ParallelFor.h"
#include "Components/DynamicMeshComponent.h"
#include "Selections/MeshConnectedComponents.h"
#include "ShapeApproximation/MeshSimpleShapeApproximation.h"
#include "Physics/PhysicsDataCollection.h"
#include "PhysicsEngine/BodySetup.h"
#include "StaticMeshResources.h"

#include "Generators/GridBoxMeshGenerator.h"

#include "Operations/MeshBoolean.h"
#include "Operations/MeshSelfUnion.h"
#include "MeshBoundaryLoops.h"
#include "Operations/MinimalHoleFiller.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

using namespace UE::Geometry;

#define LOCTEXT_NAMESPACE "UVFGeometryFunctions"

// from GeometryScript/CollisionFunctions.cpp UELocal::ComputeCollisionFromMesh()
namespace ViewFinder
{
	void ComputeCollisionFromMesh(
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
}

UDynamicMesh *UVFGeometryFunctions::SetDynamicMeshCollisionFromMesh(
	UDynamicMesh *FromDynamicMesh,
	UDynamicMeshComponent *ToDynamicMeshComponent,
	FVF_GeometryScriptCollisionFromMeshOptions Options)
{
	check(FromDynamicMesh);
	check(ToDynamicMeshComponent);

	FKAggregateGeom NewCollision;
	FromDynamicMesh->ProcessMesh([&](const FDynamicMesh3 &ReadMesh)
								 { ViewFinder::ComputeCollisionFromMesh(ReadMesh, NewCollision, Options); });

#if WITH_EDITOR
	if (Options.bEmitTransaction && GEditor)
	{
		GEditor->BeginTransaction(LOCTEXT("UpdateDynamicMesh", "Set Simple Collision"));

		ToDynamicMeshComponent->Modify();
	}
#endif

#if WITH_EDITOR
	if (Options.bEmitTransaction)
	{
		UBodySetup *BodySetup = ToDynamicMeshComponent->GetBodySetup();
		if (BodySetup != nullptr)
		{
			BodySetup->Modify();
		}
	}
#endif

	ToDynamicMeshComponent->SetSimpleCollisionShapes(NewCollision, true /*bUpdateCollision*/);

#if WITH_EDITOR
	if (Options.bEmitTransaction && GEditor)
	{
		GEditor->EndTransaction();
	}
#endif

	return FromDynamicMesh;
}

#include "Engine/StaticMesh.h"
#include "StaticMeshLODResourcesToDynamicMesh.h"

UDynamicMesh *UVFGeometryFunctions::CopyMeshFromStaticMesh(
	UStaticMesh *FromStaticMeshAsset,
	UDynamicMesh *ToDynamicMesh,
	FVF_GeometryScriptCopyMeshFromAssetOptions AssetOptions,
	FVF_GeometryScriptMeshReadLOD RequestedLOD)
{
	check(FromStaticMeshAsset);
	check(ToDynamicMesh);

	// 要求Runtime, 所以只使用CopyMeshFromStaticMesh_RenderData()
	// from GeometryScript/MeshAssetFunctions.cpp CopyMeshFromStaticMesh_RenderData()
	check(
		RequestedLOD.LODType == EVF_GeometryScriptLODType::MaxAvailable ||
		RequestedLOD.LODType == EVF_GeometryScriptLODType::RenderData);

	if (!ensure(FromStaticMeshAsset->bAllowCPUAccess))
	{
		UE_LOG(LogTemp, Warning, TEXT("Mesh %s bAllowCPUAccess needs to be true."), *FromStaticMeshAsset->GetName());
#if !WITH_EDITOR
		return ToDynamicMesh;
#endif
	}

	int32 UseLODIndex = FMath::Clamp(RequestedLOD.LODIndex, 0, FromStaticMeshAsset->GetNumLODs() - 1);

	const FStaticMeshLODResources* LODResources = nullptr;
	if (FStaticMeshRenderData* RenderData = FromStaticMeshAsset->GetRenderData())
	{
		LODResources = &RenderData->LODResources[UseLODIndex];
	}
	check(LODResources);

	FStaticMeshLODResourcesToDynamicMesh::ConversionOptions ConvertOptions;
#if WITH_EDITOR
	// respect BuildScale build setting
	const FMeshBuildSettings& LODBuildSettings = FromStaticMeshAsset->GetSourceModel(UseLODIndex).BuildSettings;
	ConvertOptions.BuildScale = (FVector3d)LODBuildSettings.BuildScale3D;
#endif

	FDynamicMesh3 NewMesh;
	FStaticMeshLODResourcesToDynamicMesh Converter;
	bool Result = Converter.Convert(LODResources, ConvertOptions, NewMesh);
	if (!Result)
		UE_LOG(LogTemp, Warning, TEXT("UVFGeometryFunctions::CopyMeshFromStaticMesh() may Get something wrong at Converter."));

	ToDynamicMesh->SetMesh(MoveTemp(NewMesh));
	return ToDynamicMesh;
}

UDynamicMesh *UVFGeometryFunctions::SetVertexPosition(
	UDynamicMesh *TargetMesh,
	int VertexID,
	FVector NewPosition,
	bool &bIsValidVertex,
	bool bDeferChangeNotifications)
{
	bIsValidVertex = false;
	if (TargetMesh)
	{
		TargetMesh->EditMesh([&](FDynamicMesh3 &EditMesh)
							 {
			if (EditMesh.IsVertex(VertexID))
			{
				bIsValidVertex = true;
				EditMesh.SetVertex(VertexID, (FVector3d)NewPosition);
			} }, EDynamicMeshChangeType::GeneralEdit, EDynamicMeshAttributeChangeFlags::Unknown, bDeferChangeNotifications);
	}
	return TargetMesh;
}

UDynamicMesh *UVFGeometryFunctions::ApplyMeshBoolean(
	UDynamicMesh *TargetMesh,
	FTransform TargetTransform,
	UDynamicMesh *ToolMesh,
	FTransform ToolTransform,
	EVF_GeometryScriptBooleanOperation Operation,
	FVF_GeometryScriptMeshBooleanOptions Options)
{
	check(TargetMesh);
	check(ToolMesh);

	FMeshBoolean::EBooleanOp ApplyOperation = FMeshBoolean::EBooleanOp::Union;
	switch (Operation)
	{
	case EVF_GeometryScriptBooleanOperation::Intersection:
		ApplyOperation = FMeshBoolean::EBooleanOp::Intersect;
		break;
	case EVF_GeometryScriptBooleanOperation::Subtract:
		ApplyOperation = FMeshBoolean::EBooleanOp::Difference;
		break;
	case EVF_GeometryScriptBooleanOperation::Union:
		ApplyOperation = FMeshBoolean::EBooleanOp::Union;
		break;
	}

	FDynamicMesh3 NewResultMesh;
	bool bSuccess = false;
	TArray<int> NewBoundaryEdges;

	TargetMesh->ProcessMesh([&](const FDynamicMesh3 &Mesh1)
							{ ToolMesh->ProcessMesh([&](const FDynamicMesh3 &Mesh2)
													{
			FMeshBoolean MeshBoolean(
				&Mesh1, (FTransformSRT3d)TargetTransform,
				&Mesh2, (FTransformSRT3d)ToolTransform,
				&NewResultMesh, ApplyOperation);
			MeshBoolean.bPutResultInInputSpace = true;
			MeshBoolean.bSimplifyAlongNewEdges = Options.bSimplifyOutput;
			bSuccess = MeshBoolean.Compute();
			NewBoundaryEdges = MoveTemp(MeshBoolean.CreatedBoundaryEdges); }); });

	if (!bSuccess) // bSuccess经常为False, 但实际并不影响
		UE_LOG(LogTemp, Warning, TEXT("UVFGeometryFunctions::ApplyMeshBoolean() may Get something wrong at ProcessMesh."));

	// 逆Transform
	MeshTransforms::ApplyTransformInverse(NewResultMesh, (FTransformSRT3d)TargetTransform, true);

	// 修复边界
	if (NewBoundaryEdges.Num() > 0 && Options.bFillHoles)
	{
		FMeshBoundaryLoops OpenBoundary(&NewResultMesh, false);
		TSet<int> ConsiderEdges(NewBoundaryEdges);
		OpenBoundary.EdgeFilterFunc = [&ConsiderEdges](int EID)
		{
			return ConsiderEdges.Contains(EID);
		};
		OpenBoundary.Compute();

		for (FEdgeLoop &Loop : OpenBoundary.Loops)
		{
			FMinimalHoleFiller Filler(&NewResultMesh, Loop);
			Filler.Fill();
		}
	}

	TargetMesh->SetMesh(MoveTemp(NewResultMesh));

	return TargetMesh;
}

UDynamicMesh *UVFGeometryFunctions::ApplyMeshSelfUnion(
	UDynamicMesh *TargetMesh,
	FVF_GeometryScriptMeshSelfUnionOptions Options)
{
	check(TargetMesh);

	bool bSuccess = false;
	TArray<int> NewBoundaryEdges;
	TargetMesh->EditMesh([&](FDynamicMesh3 &EditMesh)
						 {
		FMeshSelfUnion Union(&EditMesh);
		Union.WindingThreshold = FMath::Clamp(Options.WindingThreshold, 0.0f, 1.0f);
		Union.bTrimFlaps = Options.bTrimFlaps;
		Union.bSimplifyAlongNewEdges = Options.bSimplifyOutput;
		Union.SimplificationAngleTolerance = Options.SimplifyPlanarTolerance;
		bSuccess = Union.Compute();
		NewBoundaryEdges = MoveTemp(Union.CreatedBoundaryEdges); }, EDynamicMeshChangeType::GeneralEdit, EDynamicMeshAttributeChangeFlags::Unknown, false);

	if (!bSuccess)
		UE_LOG(LogTemp, Warning, TEXT("UVFGeometryFunctions::ApplyMeshSelfUnion() fails to EditMesh."));

	if (NewBoundaryEdges.Num() > 0 && Options.bFillHoles)
	{
		TargetMesh->EditMesh([&](FDynamicMesh3 &EditMesh)
							 {
			FMeshBoundaryLoops OpenBoundary(&EditMesh, false);
			TSet<int> ConsiderEdges(NewBoundaryEdges);
			OpenBoundary.EdgeFilterFunc = [&ConsiderEdges](int EID)
			{
				return ConsiderEdges.Contains(EID);
			};
			OpenBoundary.Compute();

			for (FEdgeLoop& Loop : OpenBoundary.Loops)
			{
				FMinimalHoleFiller Filler(&EditMesh, Loop);
				Filler.Fill();
			} }, EDynamicMeshChangeType::GeneralEdit, EDynamicMeshAttributeChangeFlags::Unknown, false);
	}

	return TargetMesh;
}

UDynamicMesh *UVFGeometryFunctions::AppendFrustum(
	UDynamicMesh *TargetMesh,
	FVF_GeometryScriptPrimitiveOptions PrimitiveOptions,
	float Angle,
	float AspectRatio,
	float StartDis,
	float EndDis)
{
	check(TargetMesh);
	// 实现: 从简单盒上修改点位置制成视锥.
	// 更好的做法是写一个FMeshShapeGenerator.

	// 生成盒状
	// from UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendBox()
	float DimensionX = 100.0f, DimensionY = 100.0f, DimensionZ = 100.0f;
	int32 StepsX = 0, StepsY = 0, StepsZ = 0;

	UE::Geometry::FAxisAlignedBox3d ConvertBox(
		FVector3d(-DimensionX / 2, -DimensionY / 2, 0),
		FVector3d(DimensionX / 2, DimensionY / 2, DimensionZ));
	FGridBoxMeshGenerator GridBoxGenerator;
	GridBoxGenerator.Box = UE::Geometry::FOrientedBox3d(ConvertBox);
	GridBoxGenerator.EdgeVertices = FIndex3i(FMath::Max(0, StepsX), FMath::Max(0, StepsY), FMath::Max(0, StepsZ));
	GridBoxGenerator.Generate();

	FVector3d OriginShift = FVector3d(0, 0, -DimensionZ / 2);
	// from UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendPrimitive()
	TargetMesh->EditMesh([&](FDynamicMesh3 &EditMesh)
						 {
			EditMesh.Copy(&GridBoxGenerator);
			// from UGeometryScriptLibrary_MeshPrimitiveFunctions.cpp ApplyPrimitiveOptionsToMesh()
			MeshTransforms::Translate(EditMesh, OriginShift);
			MeshTransforms::ApplyTransform(EditMesh, (FTransformSRT3d)FTransform::Identity, true);
			for (int32 tid : EditMesh.TriangleIndicesItr())
			{
				EditMesh.SetTriangleGroup(tid, 0);
			} }, EDynamicMeshChangeType::GeneralEdit, EDynamicMeshAttributeChangeFlags::Unknown, false);

	// 计算并放置视锥各点的位置
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
		SetVertexPosition(TargetMesh, i, Positions[i], Success, i != 7);
	}

	return TargetMesh;
}

#include "Components/SkinnedMeshComponent.h"
#include "Components/BrushComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ConversionUtils/VolumeToDynamicMesh.h"
#include "ConversionUtils/SkinnedMeshToDynamicMesh.h"
#include "ConversionUtils/SplineComponentDeformDynamicMesh.h"

UDynamicMesh *UVFGeometryFunctions::CopyMeshFromComponent(
	UPrimitiveComponent *Component,
	UDynamicMesh *ToDynamicMesh,
	FVF_GeometryScriptCopyMeshFromComponentOptions Options,
	bool bTransformToWorld)
{
	bool bSuccess = false;
	FTransform LocalToWorld = FTransform::Identity;

	if (USkinnedMeshComponent *SkinnedMeshComponent = Cast<USkinnedMeshComponent>(Component))
	{
		LocalToWorld = SkinnedMeshComponent->GetComponentTransform();

		const int32 NumLODs = SkinnedMeshComponent->GetNumLODs();
		const int32 RequestedLOD = Options.RequestedLOD.LODIndex;
		ensure(RequestedLOD >= 0);
		ensure(RequestedLOD <= NumLODs - 1);

		USkinnedAsset *SkinnedAsset = SkinnedMeshComponent->GetSkinnedAsset();
		if (ensure(SkinnedAsset))
		{
			FDynamicMesh3 NewMesh;
			UE::Conversion::SkinnedMeshComponentToDynamicMesh(*SkinnedMeshComponent, NewMesh, RequestedLOD, Options.bWantTangents);
			NewMesh.DiscardTriangleGroups();
			ToDynamicMesh->SetMesh(MoveTemp(NewMesh));
			bSuccess = true;
		}
	}
	else if (USplineMeshComponent *SplineMeshComponent = Cast<USplineMeshComponent>(Component))
	{
		LocalToWorld = SplineMeshComponent->GetComponentTransform();
		UStaticMesh *StaticMesh = SplineMeshComponent->GetStaticMesh();
		if (ensure(StaticMesh))
		{
			FVF_GeometryScriptCopyMeshFromAssetOptions AssetOptions;
			AssetOptions.bApplyBuildSettings = (Options.bWantNormals || Options.bWantTangents);
			AssetOptions.bRequestTangents = Options.bWantTangents;
			UVFGeometryFunctions::CopyMeshFromStaticMesh(
				StaticMesh, ToDynamicMesh, AssetOptions, Options.RequestedLOD);

			// deform the dynamic mesh and its tangent space with the spline
			constexpr bool bUpdateTangentSpace = true;
			UE::Geometry::SplineDeformDynamicMesh(*SplineMeshComponent, ToDynamicMesh->GetMeshRef(), bUpdateTangentSpace);
			bSuccess = true;
		}
	}
	else if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component))
	{
		LocalToWorld = StaticMeshComponent->GetComponentTransform();
		UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
		if (ensure(StaticMesh))
		{
			FVF_GeometryScriptCopyMeshFromAssetOptions AssetOptions;
			AssetOptions.bApplyBuildSettings = (Options.bWantNormals || Options.bWantTangents);
			AssetOptions.bRequestTangents = Options.bWantTangents;
			UVFGeometryFunctions::CopyMeshFromStaticMesh(
				StaticMesh, ToDynamicMesh, AssetOptions, Options.RequestedLOD);

			// if we have an ISMC, append instances
			if (UInstancedStaticMeshComponent* ISMComponent = Cast<UInstancedStaticMeshComponent>(StaticMeshComponent))
			{
				FDynamicMesh3 InstancedMesh;
				ToDynamicMesh->EditMesh([&](FDynamicMesh3& EditMesh) { InstancedMesh = MoveTemp(EditMesh); EditMesh.Clear(); },
					EDynamicMeshChangeType::MeshChange, EDynamicMeshAttributeChangeFlags::Unknown, true);

				FDynamicMesh3 AccumMesh;
				AccumMesh.EnableMatchingAttributes(InstancedMesh);
				FDynamicMeshEditor Editor(&AccumMesh);
				FMeshIndexMappings Mappings;

				int32 NumInstances = ISMComponent->GetInstanceCount();
				for (int32 InstanceIdx = 0; InstanceIdx < NumInstances; ++InstanceIdx)
				{
					if (ISMComponent->IsValidInstance(InstanceIdx))
					{
						FTransform InstanceTransform;
						ISMComponent->GetInstanceTransform(InstanceIdx, InstanceTransform, /*bWorldSpace=*/false);
						FTransformSRT3d XForm(InstanceTransform);

						Mappings.Reset();
						Editor.AppendMesh(&InstancedMesh, Mappings,
							[&](int, const FVector3d& Position) { return XForm.TransformPosition(Position); },
							[&](int, const FVector3d& Normal) { return XForm.TransformNormal(Normal); });
					}
				}

				ToDynamicMesh->EditMesh([&](FDynamicMesh3& EditMesh) { EditMesh = MoveTemp(AccumMesh); },
					EDynamicMeshChangeType::MeshChange, EDynamicMeshAttributeChangeFlags::Unknown, true);
			}
			bSuccess = true;
		}
	}
	else if (UDynamicMeshComponent* DynamicMeshComponent = Cast<UDynamicMeshComponent>(Component))
	{
		LocalToWorld = DynamicMeshComponent->GetComponentTransform();
		UDynamicMesh* CopyDynamicMesh = DynamicMeshComponent->GetDynamicMesh();
		if (ensure(CopyDynamicMesh))
		{
			CopyDynamicMesh->ProcessMesh([&](const FDynamicMesh3& Mesh)
			{
				ToDynamicMesh->SetMesh(Mesh);
			});
			bSuccess = true;
		}
	}
	else if (UBrushComponent* BrushComponent = Cast<UBrushComponent>(Component))
	{
		LocalToWorld = BrushComponent->GetComponentTransform();

		UE::Conversion::FVolumeToMeshOptions VolOptions;
		VolOptions.bMergeVertices = true;
		VolOptions.bAutoRepairMesh = true;
		VolOptions.bOptimizeMesh = true;
		VolOptions.bSetGroups = true;

		FDynamicMesh3 ConvertedMesh(EMeshComponents::FaceGroups);
		UE::Conversion::BrushComponentToDynamicMesh(BrushComponent, ConvertedMesh, VolOptions);

		// compute normals for current polygroup topology
		ConvertedMesh.EnableAttributes();
		if (Options.bWantNormals)
		{
			FDynamicMeshNormalOverlay* Normals = ConvertedMesh.Attributes()->PrimaryNormals();
			FMeshNormals::InitializeOverlayTopologyFromFaceGroups(&ConvertedMesh, Normals);
			FMeshNormals::QuickRecomputeOverlayNormals(ConvertedMesh);
		}
		ToDynamicMesh->SetMesh(MoveTemp(ConvertedMesh));
		bSuccess = true;
	}

	if (!bSuccess)
		UE_LOG(LogTemp, Warning, TEXT("UVFGeometryFunctions::CopyMeshFromComponent() doesn't success."));

	// transform mesh to world
	if (bSuccess && bTransformToWorld)
	{
		ToDynamicMesh->EditMesh([&](FDynamicMesh3 &EditMesh)
								{ MeshTransforms::ApplyTransform(EditMesh, (FTransformSRT3d)LocalToWorld, true); },
								EDynamicMeshChangeType::GeneralEdit, EDynamicMeshAttributeChangeFlags::Unknown, false);
	}

	return ToDynamicMesh;
}

#undef LOCTEXT_NAMESPACE