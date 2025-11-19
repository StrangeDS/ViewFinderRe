// Copyright StrangeDS. All Rights Reserved.

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
	// Spawn StandIn for actor
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static AActor *ReplaceWithStandIn(
		AActor *OriginalActor,
		TSubclassOf<AActor> StandInActorClass);

	// Find UVFHelperComponents under the Actors, return mapping
	template <typename T = UPrimitiveComponent>
	static void GetCompsToHelpersMapping(
		TArray<T *> &Components,
		TMap<UPrimitiveComponent *, UVFHelperComponent *> &Map);

	// Find UVFHelperComponents under the Actors, return mapping. Blueprint support.
	UFUNCTION(BlueprintCallable, Category = "ViewFinder", meta = (DisplayName = "GetCompsToHelpersMapping"))
	static void K2_GetCompsToHelpersMapping(
		UPARAM(ref) TArray<UPrimitiveComponent *> &Components,
		UPARAM(ref) TMap<UPrimitiveComponent *, UVFHelperComponent *> &Map);

	// FTransform Lerp including Scale
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static FTransform TransformLerp(const FTransform &Original, const FTransform &Target, float delta);

	// FTransform Lerp excluding Scale
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