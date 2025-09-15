#include "VFPCatcherFunctions.h"

#include "Engine/World.h"

#include "VFDynamicMeshComponent.h"
#include "VFGeometryFunctions.h"

static USceneComponent *GetComponentByName(AActor *Actor, const FName &Name)
{
	check(Actor);
	TArray<USceneComponent *> Comps;
	Actor->GetComponents<USceneComponent>(Comps);
	for (auto &Comp : Comps)
	{
		if (Comp->GetFName() == Name)
			return Comp;
	}
	check(false);
	return nullptr;
}

// 非递归拷贝Actor, Actor会在UVFDynamicMeshComponent被卸载后才进行复制, 而后组件又被装回.
// 这意味着, UVFDynamicMeshComponent上不能有复制的层级关系. 若有, 请考虑根据层级还原.
// 注意: 这也要求UVFDynamicMeshComponent不能是根组件, 是根组件会出现错误!
AActor *UVFPCatcherFunctions::CloneActorRuntime(
	AActor *Original,
	TArray<UVFDynamicMeshComponent *> &CopiedComps)
{
	if (!IsValid(Original) || !Original->GetWorld())
		return nullptr;
	UWorld *World = Original->GetWorld();

	// 卸载VFDynamicComps
	TMap<UVFDynamicMeshComponent *, USceneComponent *> Parents;
	TArray<UVFDynamicMeshComponent *> DMComps;
	TMap<UVFDynamicMeshComponent *, bool> EnableMap;
	Original->GetComponents<UVFDynamicMeshComponent>(DMComps);
	for (auto &DMComp : DMComps)
	{
		EnableMap.Emplace(DMComp, DMComp->IsEnabled());
		DMComp->SetEnabled(false);
		Parents.Add(DMComp, DMComp->GetAttachParent()); // 模拟物理的物体获得的是无效的.
		DMComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DMComp->UnregisterComponent();
		Original->RemoveInstanceComponent(DMComp);
	}

	// 复制Actor
	FActorSpawnParameters Parameters;
	Parameters.Template = Original;
	Parameters.CustomPreSpawnInitalization = [](AActor *Actor)
	{
		Actor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
	};
	AActor *Copy = World->SpawnActor<AActor>(Original->GetClass(), Parameters);
	// 手动设置transform是必要的.
	// 否则会将Original相对于其父Actor(Photo3D)的相对Transform视为绝对Transform.
	Copy->SetActorTransform(Original->GetActorTransform());

	for (auto &DMComp : DMComps)
	{
		// 还原组件
		auto Parent = Parents[DMComp];
		Original->AddInstanceComponent(DMComp);
		if (IsValid(Parent))
		{
			DMComp->AttachToComponent(
				Parent,
				FAttachmentTransformRules::KeepWorldTransform);
		}
		DMComp->RegisterComponent();
		DMComp->SetEnabled(EnableMap[DMComp]);

		UVFDynamicMeshComponent *CopiedComp =
			UVFGeometryFunctions::GetVFDMComp(Copy, DMComp->GetClass());
		CopiedComps.Add(CopiedComp);

		// 同步层级
		Copy->AddInstanceComponent(CopiedComp);
		CopiedComp->SetComponentToWorld(DMComp->GetComponentToWorld()); // 对于模拟物理的物体, 需要手动同步位置.
		if (IsValid(Parent))
		{
			CopiedComp->AttachToComponent(
				GetComponentByName(Copy, Parent->GetFName()),
				FAttachmentTransformRules::KeepWorldTransform);
		}
		CopiedComp->RegisterComponent();
		CopiedComp->Init(DMComp);
		CopiedComp->CopyMeshFromComponent(DMComp);
		CopiedComp->SetEnabled(EnableMap[DMComp]);
	}

	return Copy;
}

AActor *UVFPCatcherFunctions::K2_CloneActorRuntimeRecursive(AActor *Original)
{
	return CloneActorRuntimeRecursive<AActor>(Original);
}

// 对于已有VFDMComp的, 其实很简单, 它们其余的静态网格体在上次就已经被筛掉,
// 并不会参与后续的处理, 它们仅有的VFDMComp就是用于视锥处理的部分.
// 对于没有VFDMComp的, 它们第一次参与处理, 仅需处理与视锥overlap的基元组件.
// 每一个基元组件都对应一个VFDMComp(是否就挂载到对应组件上?)。
// 但并非挂载到源Actor上, 而是在新复制出来的Actor对应的基元组件上(如何映射?使用反射?).
// VFDMComp应当与对应基元组件拥有相同的响应函数(调用动态委托?功能都做到Actor接口上?).	暂使用Actor接口
TArray<UVFDynamicMeshComponent *> UVFPCatcherFunctions::CheckVFDMComps(
	const TArray<UPrimitiveComponent *> &Components,
	TSubclassOf<UVFDynamicMeshComponent> VFDMCompClass)
{
	check(VFDMCompClass.Get());
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("UVFPCatcherFunctions::CheckVFDMComps()"));

	TArray<UVFDynamicMeshComponent *> Result;
	for (UPrimitiveComponent *Component : Components)
	{
		AActor *Actor = Component->GetOwner();

		auto hasVFDMComp = Actor->GetComponentByClass<UVFDynamicMeshComponent>();
		if (IsValid(hasVFDMComp))
		{
			// 存在意味着处理过, 只会是VFDMComp
			if (auto VFDMComp = Cast<UVFDynamicMeshComponent>(Component))
				Result.Add(VFDMComp);
		}
		else
		{
			// 第一次参与的Actor: 挂载VFDComp, 隐藏所有基元组件.
			TArray<UPrimitiveComponent *> PrimComps;
			Actor->GetComponents<UPrimitiveComponent>(PrimComps);
			for (auto PrimComp : PrimComps)
			{
				// 只挂载被包含的组件
				if (Components.Contains(PrimComp))
				{
					UWorld *World = Actor->GetWorld();
					UVFDynamicMeshComponent *VFDMComp =
						UVFGeometryFunctions::GetVFDMComp(Actor, VFDMCompClass);

					Actor->AddInstanceComponent(VFDMComp);
					VFDMComp->AttachToComponent(PrimComp, FAttachmentTransformRules::SnapToTargetIncludingScale);
					VFDMComp->RegisterComponent();
					VFDMComp->Init(PrimComp);
					VFDMComp->ReplaceMeshForComponent(PrimComp);
					VFDMComp->SetEnabled(true);

					Result.Add(VFDMComp);
				}
			}
		}
	}
	return Result;
}

TArray<AActor *> UVFPCatcherFunctions::CopyActorsFromVFDMComps(
	UWorld *World,
	const TArray<UVFDynamicMeshComponent *> &Components,
	TArray<UVFDynamicMeshComponent *> &CopiedComps,
	bool bRetainHierarchy)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("UVFPCatcherFunctions::CopyActorFromVFDMComps()"));
	check(World);

	if (Components.Num() <= 0)
		return {};

	CopiedComps.Reset();

	TMap<AActor *, AActor *> ActorsMap;
	for (UVFDynamicMeshComponent *Component : Components)
	{
		AActor *Original = Component->GetOwner();
		// Actor的拷贝份, 纳入映射.
		if (!ActorsMap.Contains(Original))
		{
			AActor *Copy = CloneActorRuntime(Original, CopiedComps);
			ActorsMap.Add(Original, Copy);
		}
	}

	TArray<AActor *> Result;
	Result.Reserve(ActorsMap.Num());
	for (auto &[Original, Copied] : ActorsMap)
	{
		Result.Emplace(Copied);
	}

	// 修复层级关系
	if (bRetainHierarchy)
	{
		for (auto &[Original, Copy] : ActorsMap)
		{
			auto Parent = Original->GetAttachParentActor();
			if (ActorsMap.Contains(Parent))
			{
				Copy->AttachToActor(ActorsMap[Parent], FAttachmentTransformRules::KeepWorldTransform);
			}
		}
	}

	return Result;
}