#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VFCommon.h"
#include "VFHelperInterface.h"
#include "ViewFinderReSettings.h"
#include "VFFunctions.generated.h"

UCLASS(meta = (ScriptName = "VFFunctions"))
class VIEWFINDERCORE_API UVFFunctions : public UBlueprintFunctionLibrary
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

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static AActor *ReplaceWithStandIn(
		AActor *OriginalActor,
		TSubclassOf<AActor> StandInActorClass);

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

	/// @brief 查找Components对应Actor下的UVFHelperComponent组件, 映射关系返回到Map中
	template <typename T = UPrimitiveComponent>
	static void GetCompsToHelpersMapping(
		TArray<T *> &Components,
		TMap<UPrimitiveComponent *, UVFHelperComponent *> &Map);

	/// @brief 查找Components对应Actor下的UVFHelperComponent组件, 映射关系返回到Map中
	UFUNCTION(BlueprintCallable, Category = "ViewFinder", meta = (DisplayName = "GetCompsToHelpersMapping"))
	static void K2_GetCompsToHelpersMapping(
		UPARAM(ref) TArray<UPrimitiveComponent *> &Components,
		UPARAM(ref) TMap<UPrimitiveComponent *, UVFHelperComponent *> &Map);

	static FTransform TransformLerp(const FTransform &Original, const FTransform &Target, float delta);
	static FTransform TransformLerpNoScale(const FTransform &Original, const FTransform &Target, float delta);

	static bool IsEditorCreated(UObject *Object);
};

template <typename T>
inline T *UVFFunctions::CloneActorRuntimeRecursive(AActor *Original)
{
	TArray<UVFDynamicMeshComponent *> _CopiedComps; // 无用, 仅占位
	auto Res = UVFFunctions::CloneActorRuntime(Original, _CopiedComps);

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

template <typename T>
inline void UVFFunctions::GetCompsToHelpersMapping(
	UPARAM(ref) TArray<T *> &Components,
	UPARAM(ref) TMap<UPrimitiveComponent *, UVFHelperComponent *> &Map)
{
	check(T::StaticClass()->IsChildOf(UPrimitiveComponent::StaticClass()));
	for (auto It = Components.CreateIterator(); It; It++)
	{
		switch (GetDefault<UViewFinderReSettings>()->HelperGetting)
		{
		case EVFHelperGetting::ByVFHelperInterface:
		{
			// 是否实现IVFHelperInterface接口
			if (AActor *Actor = (*It)->GetOwner())
			{
				if (Actor->Implements<UVFHelperInterface>())
				{
					auto Helper = IVFHelperInterface::Execute_GetHelper(Actor);
					if (Helper)
						Map.Add(Cast<UPrimitiveComponent>(*It), Helper);
					else
						VF_LOG(Error, TEXT("%s GetHelper() is nullptr."), *Actor->GetName());
				}
			}
			break;
		}
		case EVFHelperGetting::ByGetComponentByClass:
		{
			// 是否有UVFHelperComponent组件, 更加自由
			auto Helper = (*It)->GetOwner()->GetComponentByClass<UVFHelperComponent>();
			if (Helper)
				Map.Add(Cast<UPrimitiveComponent>(*It), Helper);
			break;
		}
		default:
		{
			VF_LOG(Warning, TEXT("%s doesn't handle HelperGetting."), __FUNCTIONW__);
			break;
		}
		}
	}
}