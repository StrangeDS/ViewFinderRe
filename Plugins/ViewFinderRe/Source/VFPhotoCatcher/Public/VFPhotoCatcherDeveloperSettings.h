// Copyright 2026, StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VFDeveloperSettingsCommon.h"
#include "VFPhotoCatcherDeveloperSettings.generated.h"

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
	// Whether to generate a PlaneActor
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|PlaneActor")
	bool bGeneratePlaneActor = true;

	// Whether to generate PlaneActor at a unified frustum distance percentage
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|PlaneActor")
	bool bPlaneActorCommonRate = true;

	// Distance percentage (within the frustum) for generating PlaneActor
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|PlaneActor",
			  meta = (EditCondition = "bGeneratePlaneActor && bPlaneActorCommonRate",
					  UIMin = 0.1f, UIMax = 1.1f))
	float PlaneActorDistanceRate = 0.8f;
};