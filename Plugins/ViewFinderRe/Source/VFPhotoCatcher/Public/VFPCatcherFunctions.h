// Copyright 2026, StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VFPCommonFunctions.h"
#include "VFHelperInterface.h"
#include "GameFramework/Actor.h"
#include "VFPCatcherFunctions.generated.h"

UCLASS(meta = (ScriptName = "VFPCatcherFunctions"))
class VFPHOTOCATCHER_API UVFPCatcherFunctions : public UVFPCommonFunctions
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static AActor *CloneActorRuntime(
		AActor *Original,
		TArray<UVFDynamicMeshComponent *> &CopiedComps);

	/// @brief Duplicate Original Actors, using CloneActorRuntime, returning the corresponding type.
	template <typename T = AActor>
	static T *CloneActorRuntimeRecursive(AActor *Original);

	/// @brief Duplicate Original Actors, using CloneActorRuntime
	UFUNCTION(BlueprintCallable, Category = "ViewFinder", meta = (DisplayName = "CloneActorRuntimeRecursive"))
	static AActor *K2_CloneActorRuntimeRecursive(AActor *Original);

	/// @brief Check Comps: Replace their related primitives with VFDMComp.
	/// @param OverlapsComps
	/// @return VFDMComp array, the copied component for OverlapsComps
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static TArray<UVFDynamicMeshComponent *> CheckVFDMComps(
		const TArray<UPrimitiveComponent *> &Components,
		TSubclassOf<UVFDynamicMeshComponent> VFDMCompClass);

	/// @brief Duplicate Actors of VFDMComps
	/// @param Components VFDMComp array
	/// @param CopiedComps the result copied VFDMComps
	/// @param bRetainHierarchy Whether keep the hierarchical of Actors
	/// @return Actors duplicated
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static TArray<AActor *> CopyActorsFromVFDMComps(
		UWorld *World,
		UPARAM(ref) const TArray<UVFDynamicMeshComponent *> &Components,
		UPARAM(ref) TArray<UVFDynamicMeshComponent *> &CopiedComps,
		bool bRetainHierarchy = true);
};

template <typename T>
inline T *UVFPCatcherFunctions::CloneActorRuntimeRecursive(AActor *Original)
{
	TArray<UVFDynamicMeshComponent *> _CopiedComps; // Unused, placeholder only.
	auto Res = UVFPCatcherFunctions::CloneActorRuntime(Original, _CopiedComps);

	// Recursively process child Actors. DFS.
	TArray<AActor *> ChildActors;
	Original->GetAttachedActors(ChildActors);
	for (auto &ChildActor : ChildActors)
	{
		auto ChildCopied = CloneActorRuntimeRecursive<AActor>(ChildActor);
		ChildCopied->AttachToActor(Res, FAttachmentTransformRules::KeepWorldTransform);
	}
	return Cast<T>(Res);
}