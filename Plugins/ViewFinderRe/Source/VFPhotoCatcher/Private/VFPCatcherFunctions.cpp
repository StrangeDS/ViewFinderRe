// Copyright 2026, StrangeDS. All Rights Reserved.

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

/*
Non-recursive duplication of Actors: The Actor will be duplicated only after the UVFDynamicMeshComponent is unloaded, after which the component is then reattached.
Note: This also requires that the UVFDynamicMeshComponent cannot be the root component, which will cause errors!
*/
AActor *UVFPCatcherFunctions::CloneActorRuntime(
	AActor *Original,
	TArray<UVFDynamicMeshComponent *> &CopiedComps)
{
	if (!IsValid(Original) || !Original->GetWorld())
		return nullptr;
	UWorld *World = Original->GetWorld();

	// Unload VFDynamicComps.
	TMap<UVFDynamicMeshComponent *, USceneComponent *> Parents;
	TArray<UVFDynamicMeshComponent *> DMComps;
	TMap<UVFDynamicMeshComponent *, bool> EnableMap;
	Original->GetComponents<UVFDynamicMeshComponent>(DMComps);
	for (auto &DMComp : DMComps)
	{
		EnableMap.Emplace(DMComp, DMComp->IsEnabled());
		DMComp->SetEnabled(false);
		Parents.Add(DMComp, DMComp->GetAttachParent()); // The GetAttachParent() is invalid for simulating physics components.
		DMComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DMComp->UnregisterComponent();
		Original->RemoveInstanceComponent(DMComp);
	}

	// Duplication Actor.
	FActorSpawnParameters Parameters;
	Parameters.Template = Original;
	Parameters.CustomPreSpawnInitalization = [](AActor *Actor)
	{
		Actor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
	};
	AActor *Copy = World->SpawnActor<AActor>(Original->GetClass(), Parameters);

	/*
	Manually setting the transform is necessary.
	Otherwise, the Original Actor's relative transform(relative to Photo3D) will be treated as an absolute transform.
	*/
	Copy->SetActorTransform(Original->GetActorTransform());

	for (auto &DMComp : DMComps)
	{
		// Restore the hierarchy of VFDynamicComps.
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

		// Synchronize the hierarchy of VFDynamicComps.
		Copy->AddInstanceComponent(CopiedComp);
		CopiedComp->SetComponentToWorld(DMComp->GetComponentToWorld()); // For components simulating physics, the position must be manually synchronized.
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

/*
For actors with existing VFDMComps:
These are not undergoing processing for the first time.
Other static mesh components have already been filtered out earlier and will not participate in subsequent processing.
Only their existing VFDMComps need to be handled(for frustum bool) .

For actors without VFDMComps:
These are undergoing processing for the first time.
PrimitiveComponents overlapping with the frustum need to be processed.
Each primitive component is replaced with a corresponding VFDMComp, which is attached to the respective component.
Instead of attaching to the source Actor and then duplicating, the VFDMComp is attached (using an object pool) to the corresponding primitive component on the newly duplicated Actor.
The VFDMComp should have the same response functions as the corresponding primitive component. The Actor interface is temporarily used for this purpose.
*/
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
			// Presence indicates it has been processed and can only be VFDMComp.
			if (auto VFDMComp = Cast<UVFDynamicMeshComponent>(Component))
				Result.Add(VFDMComp);
		}
		else
		{
			// Actors participating for the first time: Mount VFDComp and use it as a replacement.
			TArray<UPrimitiveComponent *> PrimComps;
			Actor->GetComponents<UPrimitiveComponent>(PrimComps);
			for (auto PrimComp : PrimComps)
			{
				// Add to components only included.
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
		// Duplicate of the Actor, incorporated into the mapping.
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

	// Repair hierarchical relationships between Actors.
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