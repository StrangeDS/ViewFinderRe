#pragma once

#include "CoreMinimal.h"
#include "VFDeveloperSettingsCommon.h"
#include "VFPhotoCatcherDeveloperSettings.generated.h"

// PlaneActor相关设置
UCLASS(Config = ViewFinderReSettings, defaultconfig,
	   autoExpandCategories =
		   ("Settings", "Settings|PlaneActor"))
class VFPHOTOCATCHER_API UVFPhotoCatcherDeveloperSettings : public UVFDeveloperSettingsCommon
{
	GENERATED_BODY()

public:
	UVFPhotoCatcherDeveloperSettings(
		const FObjectInitializer &ObjectInitializer = FObjectInitializer::Get()) {};

public:
	// 是否生成背景的PlaneActor
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|PlaneActor")
	bool bGeneratePlaneActor = true;

	// 是否使用统一视锥距离百分比
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|PlaneActor")
	bool bPlaneActorCommonRate = true;

	// 生成背景的PlaneActor在视锥的距离(百分比)
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|PlaneActor",
			  meta = (EditCondition = "bGeneratePlaneActor && bPlaneActorCommonRate",
					  UIMin = 0.1f, UIMax = 1.1f))
	float PlaneActorDistanceRate = 0.8f;
};