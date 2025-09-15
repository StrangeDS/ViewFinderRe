#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VFLog.h"
#include "VFHelperComponent.h"
#include "VFHelperInterface.h"
#include "VFPhotoCommonDeveloperSettings.h"
#include "VFPCommonFunctions.generated.h"

UCLASS(meta = (ScriptName = "VFPCommonFunctions"))
class VFPHOTOCOMMON_API UVFPCommonFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 生成替身用于替换
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static AActor *ReplaceWithStandIn(
		AActor *OriginalActor,
		TSubclassOf<AActor> StandInActorClass);

	// 查找Components对应Actor下的UVFHelperComponent组件, 映射关系返回到Map中
	template <typename T = UPrimitiveComponent>
	static void GetCompsToHelpersMapping(
		TArray<T *> &Components,
		TMap<UPrimitiveComponent *, UVFHelperComponent *> &Map);

	// 查找Components对应Actor下的UVFHelperComponent组件, 映射关系返回到Map中, 蓝图支持
	UFUNCTION(BlueprintCallable, Category = "ViewFinder", meta = (DisplayName = "GetCompsToHelpersMapping"))
	static void K2_GetCompsToHelpersMapping(
		UPARAM(ref) TArray<UPrimitiveComponent *> &Components,
		UPARAM(ref) TMap<UPrimitiveComponent *, UVFHelperComponent *> &Map);

	// FTransform的Lerp, 包括Scale
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static FTransform TransformLerp(const FTransform &Original, const FTransform &Target, float delta);

	// FTransform的Lerp, 不包括Scale
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static FTransform TransformLerpNoScale(const FTransform &Original, const FTransform &Target, float delta);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static bool IsEditorCreated(UObject *Object);
};

template <typename T>
inline void UVFPCommonFunctions::GetCompsToHelpersMapping(
	TArray<T *> &Components,
	TMap<UPrimitiveComponent *, UVFHelperComponent *> &Map)
{
	static_assert(TIsDerivedFrom<T, UPrimitiveComponent>::IsDerived, "T must be derived from UPrimitiveComponent");

	auto HelperGetting = GetDefault<UVFPhotoCommonDeveloperSettings>()->HelperGetting;
	for (T *Comp : Components)
	{
		AActor *Actor = Comp->GetOwner();
		check(Actor);

		switch (HelperGetting)
		{
		case EVFHelperGetting::ByVFHelperInterface:
		{
			if (Actor->Implements<UVFHelperInterface>())
			{
				auto Helper = IVFHelperInterface::Execute_GetHelper(Actor);
				if (Helper)
					Map.Add(Cast<UPrimitiveComponent>(Comp), Helper);
				else
					VF_LOG(Error, TEXT("%s GetHelper() is nullptr."), *Actor->GetName());
			}
			break;
		}
		case EVFHelperGetting::ByGetComponentByClass:
		{
			UVFHelperComponent *Helper = Cast<UVFHelperComponent>(
				Actor->GetComponentByClass(UVFHelperComponent::StaticClass()));
			if (Helper)
				Map.Add(Cast<UPrimitiveComponent>(Comp), Helper);
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