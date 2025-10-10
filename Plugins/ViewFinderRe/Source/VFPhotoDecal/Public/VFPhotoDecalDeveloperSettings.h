#pragma once

#include "CoreMinimal.h"
#include "VFDeveloperSettingsCommon.h"
#include "VFPhotoDecalDeveloperSettings.generated.h"

UCLASS(Config = ViewFinderReSettings, defaultconfig,
	   autoExpandCategories =
		   ("Settings", "Settings|PhotoDecal"))
class VFPHOTODECAL_API UVFPhotoDecalDeveloperSettings : public UVFDeveloperSettingsCommon
{
	GENERATED_BODY()

public:
	UVFPhotoDecalDeveloperSettings(
		const FObjectInitializer &ObjectInitializer =FObjectInitializer::Get()) {};

public:
	// 通用贴花光照强度修正
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|PhotoDecal",
			  meta = (UIMin = 0.01f, UIMax = 2.0f))
	float PhotoDecalLightFix = 0.9f;
};