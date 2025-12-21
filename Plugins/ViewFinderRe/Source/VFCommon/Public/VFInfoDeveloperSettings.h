// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VFDeveloperSettingsCommon.h"
#include "VFInfoDeveloperSettings.generated.h"

// Display information
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

	// Subject to project README.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Informations")
	FString OpenSourceLicense = TEXT("MPL2.0");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Informations")
	FString DocsURL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Informations")
	FString Developer;
};