#include "VFFunctions.h"

#include "VFDynamicMeshComponent.h"
#include "VFDMCompPoolWorldSubsystem.h"
#include "VFStandInInterface.h"

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

static UVFDynamicMeshComponent *NewVFDMComp(UObject *Outer, const TSubclassOf<UVFDynamicMeshComponent> &Class)
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

// Actor会在UVFDynamicMeshComponent被卸载后才进行复制, 而后组件又被装回.
// 这意味着, UVFDynamicMeshComponent上不能有复制的层级关系. 若有, 请考虑根据层级还原.
// 非递归拷贝Actor, 需要递归可参考AVFPhoto2D::CopyPhoto3D()
AActor *UVFFunctions::CloneActorRuntime(
	AActor *Original,
	TArray<UVFDynamicMeshComponent *> &CopiedComps)
{
	if (!Original || !Original->GetWorld())
		return nullptr;
	UWorld *World = Original->GetWorld();

	// 卸载VFDynamicComps
	TMap<UVFDynamicMeshComponent *, USceneComponent *> Parents;
	TArray<UVFDynamicMeshComponent *> DMComps;
	Original->GetComponents<UVFDynamicMeshComponent>(DMComps);
	for (auto &DMComp : DMComps)
	{
		Parents.Add(DMComp, DMComp->GetAttachParent());
		Original->RemoveInstanceComponent(DMComp);
		DMComp->UnregisterComponent();
	}

	// 复制Actor
	FActorSpawnParameters Parameters;
	Parameters.Template = Original;
	Parameters.CustomPreSpawnInitalization = [](AActor *Actor)
	{
		Actor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
	};
	AActor *Copy = World->SpawnActor<AActor>(Original->GetClass(), Parameters);
	// 手动设置transform是必要的
	// 否则会将Original相对于其父Actor(Photo3D)的相对Transform视为绝对Transform.
	Copy->SetActorTransform(Original->GetActorTransform());

	for (auto &DMComp : DMComps)
	{
		// 还原组件
		auto Parent = Parents[DMComp];
		Original->AddInstanceComponent(DMComp);
		DMComp->AttachToComponent(Parent, FAttachmentTransformRules::SnapToTargetIncludingScale);
		DMComp->RegisterComponent();

		UVFDynamicMeshComponent *CopiedComp = NewVFDMComp(Copy, DMComp->GetClass());
		CopiedComps.Add(CopiedComp);
		// 同步层级
		Copy->AddInstanceComponent(CopiedComp);
		CopiedComp->RegisterComponent();
		CopiedComp->AttachToComponent(
			GetComponentByName(Copy, Parent->GetFName()),
			FAttachmentTransformRules::SnapToTargetIncludingScale);
		CopiedComp->CopyMeshFromComponent(DMComp);
	}

	return Copy;
}

AActor *UVFFunctions::ReplaceWithStandIn(AActor *SourceActor, TSubclassOf<AActor> StandInActorClass)
{
	auto StandIn = SourceActor->GetWorld()->SpawnActor<AActor>(
		StandInActorClass,
		SourceActor->GetActorLocation(),
		SourceActor->GetActorRotation());
	IVFStandInInterface::Execute_SetSourceActor(StandIn, SourceActor);
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
		if (hasVFDMComp)
		{
			// 存在意味着处理过, 只会是VFDMComp
			auto VFDMComp = Cast<UVFDynamicMeshComponent>(Component);
			if (VFDMComp)
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
					auto PoolSystem = World->GetSubsystem<UVFDMCompPoolWorldSubsystem>();
					UVFDynamicMeshComponent *VFDMComp = NewVFDMComp(Actor, VFDMCompClass);

					Actor->AddInstanceComponent(VFDMComp);
					VFDMComp->RegisterComponent();
					VFDMComp->AttachToComponent(PrimComp, FAttachmentTransformRules::SnapToTargetIncludingScale);
					VFDMComp->ReplaceMeshForComponent(PrimComp);

					Result.Add(VFDMComp);
				}
			}
		}
	}
	return Result;
}

FTransform UVFFunctions::TransformLerp(const FTransform &A, const FTransform &B, float delta)
{
	FRotator Rot = FMath::Lerp(A.Rotator(), B.Rotator(), delta);
	FVector Loc = FMath::Lerp(A.GetLocation(), B.GetLocation(), delta);
	FVector Scaled = FMath::Lerp(A.GetScale3D(), B.GetScale3D(), delta);
	return FTransform(Rot, Loc, Scaled);
}

TArray<AActor *> UVFFunctions::CopyActorFromVFDMComps(
	UWorld *World,
	const TArray<UVFDynamicMeshComponent *> &Components,
	TArray<UVFDynamicMeshComponent *> &CopiedComps)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("UVFFunctions::CopyActorFromVFDMComps()"));
	CopiedComps.Reset();

	check(World);
	if (Components.Num() <= 0)
		return {};

	TMap<AActor *, AActor *> ActorsMap;
	for (UVFDynamicMeshComponent *Component : Components)
	{
		AActor *Source = Component->GetOwner();
		// Actor的拷贝份, 纳入映射.
		if (!ActorsMap.Contains(Source))
		{
			AActor *Copy = CloneActorRuntime(Source, CopiedComps);
			ActorsMap.Add(Source, Copy);
		}
		AActor *Copied = ActorsMap[Source];
	}

	TArray<AActor *> Result;
	for (auto &[Source, Copied] : ActorsMap)
	{
		Result.Emplace(Copied);
	}

	return Result;
}
