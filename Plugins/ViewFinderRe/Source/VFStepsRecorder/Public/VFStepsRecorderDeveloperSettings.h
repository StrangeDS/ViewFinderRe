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
	// 回溯系统的自定义Tick间隔
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|StepsRecorder",
			  meta = (UIMin = 0.001f, UIMax = 1.0f))
	float StepsRecorderTickInterval = 0.05f;

	// 回溯系统回溯时的时间倍率
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|StepsRecorder",
			  meta = (UIMin = 1.0f, UIMax = 10.0f))
	float StepsRecorderRewindFactor = 3.0f;

	// 回溯系统回溯到上个关键点的, 计划的时间
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|StepsRecorder",
			  meta = (UIMin = 0.1f, UIMax = 5.0f))
	float StepsRecorderTimeOfRewindToLastKey = 3.0f;

	/*
	回溯系统建议的数组预分配数量
	更好的做法是写一个内存分配器: 直接给定10分, 1小时, 1天等阶梯式的内存大小.
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