#pragma once

#include "CoreMinimal.h"
#include "VFDeveloperSettingsCommon.h"
#include "VFGSGSNDeveloperSettings.generated.h"

// 视锥分段设置
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
	// 仅bUseGeometryScript为false可用, 现版本法线存在问题.
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|FrustumSegment")
	bool bUseFrustumSegmented = true;

	// 视锥分段参数, 需要重启编辑器.
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|FrustumSegment",
			  meta = (EditCondition = "bUseFrustumSegmented"))
	FVector FrustumSegmentSize = FVector(2000.0f, 2000.0f, 2000.0f);
};