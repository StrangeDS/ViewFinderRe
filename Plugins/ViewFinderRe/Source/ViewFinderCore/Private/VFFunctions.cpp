#include "VFFunctions.h"

#include "VFDynamicMeshComponent.h"
#include "VFDynamicMeshPoolWorldSubsystem.h"

// 非递归拷贝Actor, 需要递归可参考AVFPhoto2D::CopyPhoto3D()
AActor *UVFFunctions::CloneActorRuntime(AActor *Original)
{
	if (!Original || !Original->GetWorld())
		return nullptr;
	UWorld *World = Original->GetWorld();

	FActorSpawnParameters Parameters;
	Parameters.Template = Original;
	Parameters.CustomPreSpawnInitalization = [](AActor *Actor)
	{
		Actor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
	};
	AActor *Copy = World->SpawnActor<AActor>(Original->GetClass(), Parameters);
	// 手动设置transform是必要的, 否则会将Original相对于其父Actor(Photo3D)的相对Transform视为绝对Transform.
	Copy->SetActorTransform(Original->GetActorTransform());
	return Copy;
}

bool UVFFunctions::CheckPawnComps(TArray<UPrimitiveComponent *> &Components, TSubclassOf<AVFPawnStandIn> PawnStandInClass, bool NeedStandIn)
{
	TArray<APawn *> Pawns;

	for (auto It = Components.CreateIterator(); It; ++It)
	{
		auto Comp = *It;
		if (auto Pawn = Cast<APawn>(Comp->GetOwner()))
		{
			Pawns.AddUnique(Pawn);
			It.RemoveCurrent();
		}
	}

	if (!NeedStandIn)
		return Pawns.Num() > 0;

	for (const auto &Pawn : Pawns)
	{
		auto StandIn = Pawn->GetWorld()->SpawnActor<AVFPawnStandIn>(
			PawnStandInClass,
			Pawn->GetActorLocation(),
			Pawn->GetControlRotation());
		StandIn->SetTargetPawn(Pawn);
		Components.Add(StandIn->GetVFDMComp());
	}

	return Pawns.Num() > 0;
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
					// UWorld *World = Actor->GetWorld();
					// UVFDynamicMeshPoolWorldSubsystem *PoolSystem = World->GetSubsystem<UVFDynamicMeshPoolWorldSubsystem>();

					UVFDynamicMeshComponent *VFDMComp = NewObject<UVFDynamicMeshComponent>(Actor, VFDMCompClass.Get());

					VFDMComp->RegisterComponent();
					VFDMComp->AttachToComponent(PrimComp, FAttachmentTransformRules::SnapToTargetIncludingScale);
					Actor->AddInstanceComponent(VFDMComp);
					VFDMComp->ReplaceMeshForComponent(PrimComp);

					Result.Add(VFDMComp);
				}
			}
		}
	}
	return Result;
}

UVFDynamicMeshComponent *UVFFunctions::GetCloneVFDMComp(UVFDynamicMeshComponent *Target, AActor *Copied)
{
	TArray<UVFDynamicMeshComponent *> VFDMComps;
	Copied->GetComponents<UVFDynamicMeshComponent>(VFDMComps);
	for (const auto &VFDMComp : VFDMComps)
	{
		if (VFDMComp->GetName() == Target->GetFName())
		{
			return VFDMComp;
		}
	}
	return nullptr;
}

TArray<AActor *> UVFFunctions::CopyActorFromVFDMComps(UWorld *World, const TArray<UVFDynamicMeshComponent *> &Components, TArray<UVFDynamicMeshComponent *> &CopiedComps)
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
			AActor *Copy = CloneActorRuntime(Source);
			// Copy->AttachToActor(Photo, FAttachmentTransformRules::KeepWorldTransform);
			ActorsMap.Add(Source, Copy);
		}
		AActor *Copied = ActorsMap[Source];
	}

	for (UVFDynamicMeshComponent *Component : Components)
	{
		auto CopiedComp = GetCloneVFDMComp(Component, ActorsMap[Component->GetOwner()]);
		CopiedComp->CopyMeshFromComponent(Component);
		CopiedComps.Emplace(CopiedComp);
	}

	TArray<AActor *> Result;
	for (auto &[Source, Copied] : ActorsMap)
	{
		Result.Emplace(Copied);
	}

	return Result;
}
