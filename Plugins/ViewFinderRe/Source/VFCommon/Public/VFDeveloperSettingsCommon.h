#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "VFDeveloperSettingsCommon.generated.h"

// 处于相同的Container和Category
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