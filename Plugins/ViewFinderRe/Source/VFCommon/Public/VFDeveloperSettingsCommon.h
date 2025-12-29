// Copyright 2026, StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "VFDeveloperSettingsCommon.generated.h"

// Unified Container and Category, ensuring plugin's all UDeveloperSettings reside in a single file.
UCLASS(Abstract)
class VFCOMMON_API UVFDeveloperSettingsCommon : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UVFDeveloperSettingsCommon(
		const FObjectInitializer &ObjectInitializer = FObjectInitializer::Get()) {};

	virtual FName GetContainerName() const override
	{
		return FName(TEXT("Project"));
	};

	virtual FName GetCategoryName() const override
	{
		return FName(TEXT("ViewFinderRe"));
	};
};