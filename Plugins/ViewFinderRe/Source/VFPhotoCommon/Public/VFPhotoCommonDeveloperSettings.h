#pragma once

#include "CoreMinimal.h"
#include "VFDeveloperSettingsCommon.h"
#include "VFPhotoCommonDeveloperSettings.generated.h"

// 查找Helper组件的方式
UENUM(BlueprintType)
enum class EVFHelperGetting : uint8
{
	// 是否实现IVFHelperInterface接口, 更高效
	ByVFHelperInterface,
	// 是否有UVFHelperComponent组件, 更加自由
	ByGetComponentByClass,
};

// Helper获取方式
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
	// 查找Helper组件的方式
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|HelperGetting")
	EVFHelperGetting HelperGetting = EVFHelperGetting::ByVFHelperInterface;
};