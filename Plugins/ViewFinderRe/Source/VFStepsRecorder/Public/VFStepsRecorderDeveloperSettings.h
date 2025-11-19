// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VFDeveloperSettingsCommon.h"
#include "VFStepsRecorderDeveloperSettings.generated.h"

UCLASS(Config = ViewFinderReSettings, defaultconfig,
	   autoExpandCategories =
		   ("Settings", "Settings|StepsRecorder"))
class VFSTEPSRECORDER_API UVFStepsRecorderDeveloperSettings : public UVFDeveloperSettingsCommon
{
	GENERATED_BODY()

public:
	UVFStepsRecorderDeveloperSettings(
		const FObjectInitializer &ObjectInitializer = FObjectInitializer::Get()) {};

public:
	// Custom tick interval.
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|StepsRecorder",
			  meta = (UIMin = 0.001f, UIMax = 1.0f))
	float StepsRecorderTickInterval = 0.05f;

	// Rewind time Factor.
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|StepsRecorder",
			  meta = (UIMin = 1.0f, UIMax = 10.0f))
	float StepsRecorderRewindFactor = 3.0f;

	// Scheduled time to rewind to the last key event.
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|StepsRecorder",
			  meta = (UIMin = 0.1f, UIMax = 5.0f))
	float StepsRecorderTimeOfRewindToLastKey = 3.0f;

	/*
	Recommended pre-allocation size for the backtracking system's array.
	A better approach is to implement a memory allocator: directly allocate tiered memory sizes (e.g., 10 minutes, 1 hour, 1 day).
	*/
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|StepsRecorder",
			  meta = (UIMin = 0, UIMax = 1000000))
#if WITH_EDITOR
	int StepsRecorderSizeRecommended = 60 * 60 * 10;
#else
	int StepsRecorderSizeRecommended = 60 * 60 * 20 * 30;
#endif
};