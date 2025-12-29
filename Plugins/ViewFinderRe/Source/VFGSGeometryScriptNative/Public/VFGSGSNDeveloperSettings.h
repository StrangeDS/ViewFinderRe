// Copyright 2026, StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VFDeveloperSettingsCommon.h"
#include "VFGSGSNDeveloperSettings.generated.h"

// Frustum Segmentation Settings
UCLASS(Config = ViewFinderReSettings, defaultconfig,
	   autoExpandCategories =
		   ("Settings", "Settings|FrustumSegment"))
class VFGSGEOMETRYSCRIPTNATIVE_API UVFGSGSNDeveloperSettings : public UVFDeveloperSettingsCommon
{
	GENERATED_BODY()

public:
	UVFGSGSNDeveloperSettings(
		const FObjectInitializer &ObjectInitializer = FObjectInitializer::Get()) {};

public:
	// The current version has normal vector issues.
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|FrustumSegment")
	bool bUseFrustumSegmented = true;

	// Frustum segmentation parameters require an editor restart.
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|FrustumSegment",
			  meta = (EditCondition = "bUseFrustumSegmented"))
	FVector FrustumSegmentSize = FVector(2000.0f, 2000.0f, 2000.0f);
};