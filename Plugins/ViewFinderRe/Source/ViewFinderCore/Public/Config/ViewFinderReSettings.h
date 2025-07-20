#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ViewFinderReSettings.generated.h"

USTRUCT(BlueprintType)
struct FViewFinderReDescriptor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	FString Description;

	// 参看项目文档
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	FString OpenSourceLicense = TEXT("MPL2.0");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	FString DocsURL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	FString Developer;
};

UCLASS(Config = ViewFinderReSettings, defaultconfig)
class VIEWFINDERCORE_API UViewFinderReSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UViewFinderReSettings(const FObjectInitializer &ObjectInitializer =
							  FObjectInitializer::Get());

	virtual FName GetContainerName() const;

	virtual FName GetCategoryName() const;

	virtual FName GetSectionName() const;

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static UViewFinderReSettings *Get();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static void Save(UViewFinderReSettings *Setting = nullptr);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Infomation")
	FViewFinderReDescriptor Descriptor;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Setting")
	bool bUseGeometryScript = false;
};