#include "VFGeometryFunctions.h"

#include "VFCommon.h"

// WITHOUT_GEOMETRY_SCRIPT开启后(默认), 将不依赖插件.
// 关闭宏, 将依赖插件接口(考虑到后续更新)

// 注意! 若使用插件, 需要:
// 依赖模块 GeometryScriptingCore (解注释ViewFinderCore.Build.cs line 30)
// 添加插件依赖 GeometryScripting:
// 在 ViewFinderRe.uplugin 的"Plugins"中添加
// {
// 	"Name": "GeometryScripting",
// 	"Enabled": true
// }

#define WITHOUT_GEOMETRY_SCRIPT 1

#if WITHOUT_GEOMETRY_SCRIPT
// 脱离插件使用

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
		VF_LOG(Error, TEXT("Mesh %s bAllowCPUAccess needs to be true."), *FromStaticMeshAsset->GetName());
#if !WITH_EDITOR
		return ToDynamicMesh;
#endif
	}

	int32 UseLODIndex = FMath::Clamp(RequestedLOD.LODIndex, 0, FromStaticMeshAsset->GetNumLODs() - 1);

	const FStaticMeshLODResources *LODResources = nullptr;
	if (FStaticMeshRenderData *RenderData = FromStaticMeshAsset->GetRenderData())
	{
		LODResources = &RenderData->LODResources[UseLODIndex];
	}
	check(LODResources);

	FStaticMeshLODResourcesToDynamicMesh::ConversionOptions ConvertOptions;
#if WITH_EDITOR
	// respect BuildScale build setting
	const FMeshBuildSettings &LODBuildSettings = FromStaticMeshAsset->GetSourceModel(UseLODIndex).BuildSettings;
	ConvertOptions.BuildScale = (FVector3d)LODBuildSettings.BuildScale3D;
#endif

	FDynamicMesh3 NewMesh;
	FStaticMeshLODResourcesToDynamicMesh Converter;
	bool Result = Converter.Convert(LODResources, ConvertOptions, NewMesh);
	if (!Result)
		VF_LOG(Warning, TEXT("%s may Get something wrong at Converter."), __FUNCTIONW__);

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

	// if (!bSuccess) // bSuccess经常为False, 但实际并不影响
	// 	VF_LOG(Warning, TEXT("%s may Get something wrong at ProcessMesh."), __FUNCTIONW__);

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
		VF_LOG(Warning, TEXT("%s fails to EditMesh."), __FUNCTIONW__);

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

#include "FFrustumGenerator.h"

UDynamicMesh *UVFGeometryFunctions::AppendFrustum(
	UDynamicMesh *TargetMesh,
	FVF_GeometryScriptPrimitiveOptions PrimitiveOptions,
	float ViewAngle,
	float AspectRatio,
	float StartDis,
	float EndDis)
{
	check(TargetMesh);

	UE::Geometry::Frustum::FFrustumGenerator Generator;
	Generator.VerticalFOV = ViewAngle;
	Generator.AspectRatio = AspectRatio;
	Generator.NearPlaneDis = StartDis;
	Generator.FarPlaneDis = EndDis;
	Generator.Generate();

	TargetMesh->EditMesh(
		[&](FDynamicMesh3 &EditMesh)
		{
			EditMesh.Copy(&Generator);
			MeshTransforms::ApplyTransform(EditMesh, (FTransformSRT3d)FTransform::Identity, true);
		},
		EDynamicMeshChangeType::GeneralEdit,
		EDynamicMeshAttributeChangeFlags::Unknown,
		false);
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
	else if (UStaticMeshComponent *StaticMeshComponent = Cast<UStaticMeshComponent>(Component))
	{
		LocalToWorld = StaticMeshComponent->GetComponentTransform();
		UStaticMesh *StaticMesh = StaticMeshComponent->GetStaticMesh();
		if (ensure(StaticMesh))
		{
			FVF_GeometryScriptCopyMeshFromAssetOptions AssetOptions;
			AssetOptions.bApplyBuildSettings = (Options.bWantNormals || Options.bWantTangents);
			AssetOptions.bRequestTangents = Options.bWantTangents;
			UVFGeometryFunctions::CopyMeshFromStaticMesh(
				StaticMesh, ToDynamicMesh, AssetOptions, Options.RequestedLOD);

			// if we have an ISMC, append instances
			if (UInstancedStaticMeshComponent *ISMComponent = Cast<UInstancedStaticMeshComponent>(StaticMeshComponent))
			{
				FDynamicMesh3 InstancedMesh;
				ToDynamicMesh->EditMesh([&](FDynamicMesh3 &EditMesh)
										{ InstancedMesh = MoveTemp(EditMesh); EditMesh.Clear(); },
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
						Editor.AppendMesh(&InstancedMesh, Mappings, [&](int, const FVector3d &Position)
										  { return XForm.TransformPosition(Position); }, [&](int, const FVector3d &Normal)
										  { return XForm.TransformNormal(Normal); });
					}
				}

				ToDynamicMesh->EditMesh([&](FDynamicMesh3 &EditMesh)
										{ EditMesh = MoveTemp(AccumMesh); },
										EDynamicMeshChangeType::MeshChange, EDynamicMeshAttributeChangeFlags::Unknown, true);
			}
			bSuccess = true;
		}
	}
	else if (UDynamicMeshComponent *DynamicMeshComponent = Cast<UDynamicMeshComponent>(Component))
	{
		LocalToWorld = DynamicMeshComponent->GetComponentTransform();
		UDynamicMesh *CopyDynamicMesh = DynamicMeshComponent->GetDynamicMesh();
		if (ensure(CopyDynamicMesh))
		{
			CopyDynamicMesh->ProcessMesh([&](const FDynamicMesh3 &Mesh)
										 { ToDynamicMesh->SetMesh(Mesh); });
			bSuccess = true;
		}
	}
	else if (UBrushComponent *BrushComponent = Cast<UBrushComponent>(Component))
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
			FDynamicMeshNormalOverlay *Normals = ConvertedMesh.Attributes()->PrimaryNormals();
			FMeshNormals::InitializeOverlayTopologyFromFaceGroups(&ConvertedMesh, Normals);
			FMeshNormals::QuickRecomputeOverlayNormals(ConvertedMesh);
		}
		ToDynamicMesh->SetMesh(MoveTemp(ConvertedMesh));
		bSuccess = true;
	}

	if (!bSuccess)
		VF_LOG(Warning, TEXT("%s doesn't success."), __FUNCTIONW__);

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

#else
// 使用插件
// 布尔操作经常报错, 但没关系, It Works Fine.
// 记得其中一个原因是: 判断bool后模型的顶点数, 为0就报错.这种情况在我们这当然有, 而且很常见.

#include "GeometryScript/CollisionFunctions.h"
#include "GeometryScript/MeshAssetFunctions.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshBooleanFunctions.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "GeometryScript/SceneUtilityFunctions.h"

// 中间层参数需要转换为插件中的参数类型
// 可考虑使用地址, memcpy(&b, &a, sizeof(A)), 但万一插件更新, 又新添了一个值呢?
// 手动转换的健壮性强得多, 但也保留宏开关
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

// 宏 MANUAL_CONVERT 结束
#endif

UDynamicMesh *UVFGeometryFunctions::SetDynamicMeshCollisionFromMesh(
	UDynamicMesh *FromDynamicMesh,
	UDynamicMeshComponent *ToDynamicMeshComponent,
	FVF_GeometryScriptCollisionFromMeshOptions Options_)
{
	FGeometryScriptCollisionFromMeshOptions Options = Convert(Options_);

	return UGeometryScriptLibrary_CollisionFunctions::SetDynamicMeshCollisionFromMesh(
		FromDynamicMesh,
		ToDynamicMeshComponent,
		Options,
		nullptr);
}

UDynamicMesh *UVFGeometryFunctions::CopyMeshFromStaticMesh(
	UStaticMesh *FromStaticMeshAsset,
	UDynamicMesh *ToDynamicMesh,
	FVF_GeometryScriptCopyMeshFromAssetOptions AssetOptions,
	FVF_GeometryScriptMeshReadLOD RequestedLOD)
{
	EGeometryScriptOutcomePins Outcome;
	return UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshFromStaticMesh(
		FromStaticMeshAsset,
		ToDynamicMesh,
		Convert(AssetOptions),
		Convert(RequestedLOD),
		Outcome,
		nullptr);
}

UDynamicMesh *UVFGeometryFunctions::SetVertexPosition(
	UDynamicMesh *TargetMesh,
	int VertexID,
	FVector NewPosition,
	bool &bIsValidVertex,
	bool bDeferChangeNotifications)
{
	return UGeometryScriptLibrary_MeshBasicEditFunctions::SetVertexPosition(
		TargetMesh,
		VertexID,
		NewPosition,
		bIsValidVertex,
		bDeferChangeNotifications);
}

UDynamicMesh *UVFGeometryFunctions::ApplyMeshBoolean(
	UDynamicMesh *TargetMesh,
	FTransform TargetTransform,
	UDynamicMesh *ToolMesh,
	FTransform ToolTransform,
	EVF_GeometryScriptBooleanOperation Operation,
	FVF_GeometryScriptMeshBooleanOptions Options)
{
	return UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshBoolean(
		TargetMesh,
		TargetTransform,
		ToolMesh,
		ToolTransform,
		Convert(Operation),
		Convert(Options),
		nullptr);
}

UDynamicMesh *UVFGeometryFunctions::ApplyMeshSelfUnion(
	UDynamicMesh *TargetMesh,
	FVF_GeometryScriptMeshSelfUnionOptions Options)
{
	return UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshSelfUnion(
		TargetMesh,
		Convert(Options),
		nullptr);
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

UDynamicMesh *UVFGeometryFunctions::CopyMeshFromComponent(
	UPrimitiveComponent *Component,
	UDynamicMesh *ToDynamicMesh,
	FVF_GeometryScriptCopyMeshFromComponentOptions Options,
	bool bTransformToWorld)
{
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

// 宏 !WITHOUT_GEOMETRY_SCRIPT 结束
#endif