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

	/// @brief 复制Original, 底层使用CloneActorRuntime, 返回对应类型
	template <typename T = AActor>
	static T *CloneActorRuntimeRecursive(AActor *Original);

	/// @brief 复制Original, 底层使用CloneActorRuntime
	UFUNCTION(BlueprintCallable, Category = "ViewFinder", meta = (DisplayName = "CloneActorRuntimeRecursive"))
	static AActor *K2_CloneActorRuntimeRecursive(AActor *Original);

	/// @brief 检验未处理过的的Actor, 处理为: 将其相关基元替换为VFDMComp, 关闭隐藏其它基元组件.
	/// @param OverlapsComps
	/// @return (复制)替换后的, 全为VFDMComp的列表.
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static TArray<UVFDynamicMeshComponent *> CheckVFDMComps(
		const TArray<UPrimitiveComponent *> &Components,
		TSubclassOf<UVFDynamicMeshComponent> VFDMCompClass);

	/// @brief 复制VFDMComp列表的Actors
	/// @param Components VFDMComp列表
	/// @param CopiedComps VFDMComp对应的复制组件
	/// @param bRetainHierarchy 是否需要保持Actor之间的层级关系
	/// @return 复制出的Actors
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
	TArray<UVFDynamicMeshComponent *> _CopiedComps; // 无用, 仅占位
	auto Res = UVFPCatcherFunctions::CloneActorRuntime(Original, _CopiedComps);

	// 递归子Actors
	TArray<AActor *> ChildActors;
	Original->GetAttachedActors(ChildActors);
	for (auto &ChildActor : ChildActors)
	{
		auto ChildCopied = CloneActorRuntimeRecursive<AActor>(ChildActor);
		ChildCopied->AttachToActor(Res, FAttachmentTransformRules::KeepWorldTransform);
	}
	return Cast<T>(Res);
}