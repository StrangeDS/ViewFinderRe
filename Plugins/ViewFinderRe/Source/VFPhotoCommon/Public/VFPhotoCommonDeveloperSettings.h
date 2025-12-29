// Copyright 2026, StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VFDeveloperSettingsCommon.h"
#include "VFPhotoCommonDeveloperSettings.generated.h"

// Method for finding Helper components
UENUM(BlueprintType)
enum class EVFHelperGetting : uint8
{
	// Implement the IVFHelperInterface interface(more efficient).
	ByVFHelperInterface,
	// Find the UVFHelperComponent component in runtime(more flexible).
	ByGetComponentByClass,
};

UCLASS(Config = ViewFinderReSettings, defaultconfig,
	   autoExpandCategories =
		   ("Settings", "Settings|HelperGetting"))
class VFPHOTOCOMMON_API UVFPhotoCommonDeveloperSettings : public UVFDeveloperSettingsCommon
{
	GENERATED_BODY()

public:
	UVFPhotoCommonDeveloperSettings(
		const FObjectInitializer &ObjectInitializer = FObjectInitializer::Get()) {};

public:
	// How to get Helper components
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|HelperGetting")
	EVFHelperGetting HelperGetting = EVFHelperGetting::ByVFHelperInterface;
};