#pragma once

#include "CoreMinimal.h"
#include "VFDeveloperSettingsCommon.h"
#include "VFInfoDeveloperSettings.generated.h"

// 信息展示
UCLASS(Config = ViewFinderReSettings, defaultconfig,
	   autoExpandCategories = ("Informations"))
class VFCOMMON_API UVFInfoDeveloperSettings : public UVFDeveloperSettingsCommon
{
	GENERATED_BODY()

public:
	UVFInfoDeveloperSettings(
		const FObjectInitializer &ObjectInitializer = FObjectInitializer::Get()) {};

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Informations")
	FString Description;

	// 以项目文档为准
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Informations")
	FString OpenSourceLicense = TEXT("MPL2.0");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Informations")
	FString DocsURL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Informations")
	FString Developer;
};