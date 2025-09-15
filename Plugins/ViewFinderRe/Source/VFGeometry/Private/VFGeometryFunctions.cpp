#include "VFGeometryFunctions.h"

#include "Engine/World.h"
#if WITH_EDITOR
#include "Editor.h"
#endif

#include "VFLog.h"

using namespace UE::Geometry;

#define LOCTEXT_NAMESPACE "UVFGeometryFunctions"

#include "VFGeometryDeveloperSettings.h"
#include "VFUObjsPoolWorldSubsystem.h"

bool UVFGeometryFunctions::IsEditorCreated(UObject *Object)
{
	return Object->HasAnyFlags(RF_WasLoaded | RF_LoadCompleted);
}

UVFDynamicMeshComponent *UVFGeometryFunctions::GetVFDMComp(
	UObject *Outer,
	const TSubclassOf<UVFDynamicMeshComponent> &Class)
{
	check(Outer && Class);
	auto World = Outer->GetWorld();
	check(World);

	UVFDynamicMeshComponent *Comp = nullptr;
	if (GetDefault<UVFGeometryDeveloperSettings>()->bUseVFDMCompsPool)
	{
		if (auto PoolSystem = World->GetSubsystem<UVFUObjsPoolWorldSubsystem>();
			ensureMsgf(PoolSystem, TEXT("%s invalid PoolSystem."), __FUNCTIONW__))
			Comp = PoolSystem->GetOrCreate<UVFDynamicMeshComponent>(Outer, Class);
	}
	else
	{
		Comp = NewObject<UVFDynamicMeshComponent>(Outer, Class);
	}

	return Comp;
}

bool UVFGeometryFunctions::ReturnVFDMComp(UObject *Obj)
{
	check(Obj);
	auto World = Obj->GetWorld();
	check(World);

	if (GetDefault<UVFGeometryDeveloperSettings>()->bUseVFDMCompsPool)
	{
		if (auto PoolSystem = World->GetSubsystem<UVFUObjsPoolWorldSubsystem>();
			ensureMsgf(PoolSystem, TEXT("%s invalid PoolSystem."), __FUNCTIONW__))
			return PoolSystem->Return(Obj);
	}
	return false;
}

#include "VFGeometryStrategyInterface.h"

UObject *UVFGeometryFunctions::GetGeometryStrategyCDO()
{
	auto GBSettings = GetDefault<UVFGeometryDeveloperSettings>();
	check(GBSettings->GeometryStrategyClass);
	return GBSettings->GeometryStrategyClass->GetDefaultObject();
}

UDynamicMesh *UVFGeometryFunctions::SetDynamicMeshCollisionFromMesh(
	UDynamicMesh *FromDynamicMesh,
	UDynamicMeshComponent *ToDynamicMeshComponent,
	FVF_GeometryScriptCollisionFromMeshOptions Options)
{
	check(FromDynamicMesh);
	check(ToDynamicMeshComponent);

	return IVFGeometryStrategyInterface::Execute_SetDynamicMeshCollisionFromMesh(
		GetGeometryStrategyCDO(),
		FromDynamicMesh,
		ToDynamicMeshComponent,
		Options);
}

#include "Engine/StaticMesh.h"

UDynamicMesh *UVFGeometryFunctions::CopyMeshFromStaticMesh(
	UStaticMesh *FromStaticMeshAsset,
	UDynamicMesh *ToDynamicMesh,
	FVF_GeometryScriptCopyMeshFromAssetOptions AssetOptions,
	FVF_GeometryScriptMeshReadLOD RequestedLOD)
{
	check(FromStaticMeshAsset);
	check(ToDynamicMesh);

	return IVFGeometryStrategyInterface::Execute_CopyMeshFromStaticMesh(
		GetGeometryStrategyCDO(),
		FromStaticMeshAsset,
		ToDynamicMesh,
		AssetOptions,
		RequestedLOD);
}

UDynamicMesh *UVFGeometryFunctions::SetVertexPosition(
	UDynamicMesh *TargetMesh,
	int VertexID,
	FVector NewPosition,
	bool &bIsValidVertex,
	bool bDeferChangeNotifications)
{
	check(TargetMesh);

	return IVFGeometryStrategyInterface::Execute_SetVertexPosition(
		GetGeometryStrategyCDO(),
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
	check(TargetMesh);
	check(ToolMesh);

	return IVFGeometryStrategyInterface::Execute_ApplyMeshBoolean(
		GetGeometryStrategyCDO(),
		TargetMesh,
		TargetTransform,
		ToolMesh,
		ToolTransform,
		Operation,
		Options);
}

UDynamicMesh *UVFGeometryFunctions::ApplyMeshSelfUnion(
	UDynamicMesh *TargetMesh,
	FVF_GeometryScriptMeshSelfUnionOptions Options)
{
	check(TargetMesh);

	return IVFGeometryStrategyInterface::Execute_ApplyMeshSelfUnion(
		GetGeometryStrategyCDO(),
		TargetMesh,
		Options);
}

UDynamicMesh *UVFGeometryFunctions::AppendFrustum(
	UDynamicMesh *TargetMesh,
	FVF_GeometryScriptPrimitiveOptions PrimitiveOptions,
	float ViewAngle,
	float AspectRatio,
	float StartDis,
	float EndDis)
{
	check(TargetMesh);

	return IVFGeometryStrategyInterface::Execute_AppendFrustum(
		GetGeometryStrategyCDO(),
		TargetMesh,
		PrimitiveOptions,
		ViewAngle,
		AspectRatio,
		StartDis,
		EndDis);
}

UDynamicMesh *UVFGeometryFunctions::CopyMeshFromComponent(
	UPrimitiveComponent *Component,
	UDynamicMesh *ToDynamicMesh,
	FVF_GeometryScriptCopyMeshFromComponentOptions Options,
	bool bTransformToWorld)
{
	check(Component);
	check(ToDynamicMesh);

	return IVFGeometryStrategyInterface::Execute_CopyMeshFromComponent(
		GetGeometryStrategyCDO(),
		Component,
		ToDynamicMesh,
		Options,
		bTransformToWorld);
}

#undef LOCTEXT_NAMESPACE