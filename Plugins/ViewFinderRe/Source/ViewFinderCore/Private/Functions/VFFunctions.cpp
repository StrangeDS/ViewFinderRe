#include "VFFunctions.h"

#include "Engine/World.h"

#include "VFDynamicMeshComponent.h"
#include "VFDMCompPoolWorldSubsystem.h"
#include "VFStandInInterface.h"
#include "VFHelperComponent.h"

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

static UVFDynamicMeshComponent *NewVFDMComp(
	UObject *Outer,
	const TSubclassOf<UVFDynamicMeshComponent> &Class)
{
	check(Outer && Class);
	auto World = Outer->GetWorld();
	check(World);

	UVFDynamicMeshComponent *Comp = nullptr;
	if (auto CompsPool = World->GetSubsystem<UVFDMCompPoolWorldSubsystem>())
	{
		Comp = CompsPool->GetOrCreateComp(Outer, Class);
	}
	else
	{
		Comp = NewObject<UVFDynamicMeshComponent>(Outer, Class);
	}
	return Comp;
}

// 非递归拷贝Actor, Actor会在UVFDynamicMeshComponent被卸载后才进行复制, 而后组件又被装回.
// 这意味着, UVFDynamicMeshComponent上不能有复制的层级关系. 若有, 请考虑根据层级还原.
// 注意: 这也要求UVFDynamicMeshComponent不能是根组件, 是根组件会出现错误!
AActor *UVFFunctions::CloneActorRuntime(
	AActor *Original,
	TArray<UVFDynamicMeshComponent *> &CopiedComps)
{
	if (!IsValid(Original) || !Original->GetWorld())
		return nullptr;
	UWorld *World = Original->GetWorld();

	// 卸载VFDynamicComps
	TMap<UVFDynamicMeshComponent *, USceneComponent *> Parents;
	TArray<UVFDynamicMeshComponent *> DMComps;
	Original->GetComponents<UVFDynamicMeshComponent>(DMComps);
	for (auto &DMComp : DMComps)
	{
		DMComp->SetEnabled(false);
		Parents.Add(DMComp, DMComp->GetAttachParent());
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
		DMComp->AttachToComponent(Parent, FAttachmentTransformRules::SnapToTargetIncludingScale);
		DMComp->RegisterComponent();
		DMComp->SetEnabled(true);

		UVFDynamicMeshComponent *CopiedComp = NewVFDMComp(Copy, DMComp->GetClass());
		CopiedComps.Add(CopiedComp);
		// 同步层级
		Copy->AddInstanceComponent(CopiedComp);
		CopiedComp->AttachToComponent(
			GetComponentByName(Copy, Parent->GetFName()),
			FAttachmentTransformRules::SnapToTargetIncludingScale);
		CopiedComp->RegisterComponent();
		CopiedComp->Init(DMComp);
		CopiedComp->CopyMeshFromComponent(DMComp);
	}

	return Copy;
}

AActor *UVFFunctions::K2_CloneActorRuntimeRecursive(AActor *Original)
{
	return CloneActorRuntimeRecursive<AActor>(Original);
}

AActor *UVFFunctions::ReplaceWithStandIn(AActor *OriginalActor, TSubclassOf<AActor> StandInActorClass)
{
	auto Transform = OriginalActor->GetActorTransform();
	auto StandIn = OriginalActor->GetWorld()->SpawnActorDeferred<AActor>(
		StandInActorClass,
		Transform);
	IVFStandInInterface::Execute_SetOriginalActor(StandIn, OriginalActor);
	StandIn->FinishSpawning(Transform);
	return StandIn;
}

// 对于已有VFDMComp的, 其实很简单, 它们其余的静态网格体在上次就已经被筛掉,
// 并不会参与后续的处理, 它们仅有的VFDMComp就是用于视锥处理的部分.
// 对于没有VFDMComp的, 它们第一次参与处理, 仅需处理与视锥overlap的基元组件.
// 每一个基元组件都对应一个VFDMComp(是否就挂载到对应组件上?)。
// 但并非挂载到源Actor上, 而是在新复制出来的Actor对应的基元组件上(如何映射?使用反射?).
// VFDMComp应当与对应基元组件拥有相同的响应函数(调用动态委托?功能都做到Actor接口上?).	暂使用Actor接口
TArray<UVFDynamicMeshComponent *> UVFFunctions::CheckVFDMComps(
	const TArray<UPrimitiveComponent *> &Components,
	TSubclassOf<UVFDynamicMeshComponent> VFDMCompClass)
{
	check(VFDMCompClass.Get());
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("UVFFunctions::CheckVFDMComps()"));

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
					UVFDynamicMeshComponent *VFDMComp = NewVFDMComp(Actor, VFDMCompClass);

					Actor->AddInstanceComponent(VFDMComp);
					VFDMComp->AttachToComponent(PrimComp, FAttachmentTransformRules::SnapToTargetIncludingScale);
					VFDMComp->RegisterComponent();
					VFDMComp->Init(PrimComp);
					VFDMComp->ReplaceMeshForComponent(PrimComp);

					Result.Add(VFDMComp);
				}
			}
		}
	}
	return Result;
}

void UVFFunctions::K2_GetCompsToHelpersMapping(UPARAM(ref) TArray<UPrimitiveComponent *> &Components, UPARAM(ref) TMap<UPrimitiveComponent *, UVFHelperComponent *> &Map)
{
	GetCompsToHelpersMapping<UPrimitiveComponent>(Components, Map);
}

FTransform UVFFunctions::TransformLerp(const FTransform &Original, const FTransform &Target, float delta)
{
	FRotator Rot = FMath::Lerp(Original.Rotator(), Target.Rotator(), delta);
	FVector Loc = FMath::Lerp(Original.GetLocation(), Target.GetLocation(), delta);
	return FTransform(Rot, Loc, Original.GetScale3D());
}

FTransform UVFFunctions::TransformLerpNoScale(const FTransform &Original, const FTransform &Target, float delta)
{
	FRotator Rot = FMath::Lerp(Original.Rotator(), Target.Rotator(), delta);
	FVector Loc = FMath::Lerp(Original.GetLocation(), Target.GetLocation(), delta);
	return FTransform(Rot, Loc, Original.GetScale3D());
}

TArray<AActor *> UVFFunctions::CopyActorsFromVFDMComps(
	UWorld *World,
	const TArray<UVFDynamicMeshComponent *> &Components,
	TArray<UVFDynamicMeshComponent *> &CopiedComps,
	bool bRetainHierarchy)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("UVFFunctions::CopyActorFromVFDMComps()"));
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