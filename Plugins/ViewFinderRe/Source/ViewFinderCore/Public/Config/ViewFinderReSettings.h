#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "VFSettingHeaders.h"
#include "Components/DynamicMeshComponent.h"
#include "ViewFinderReSettings.generated.h"

class UVFDynamicMeshComponent;

UCLASS(Config = ViewFinderReSettings, defaultconfig,
	   autoExpandCategories =
		   ("Informations", "Settings", "Settings|Geometry",
			"Settings|StepsRecorder", "Settings|VFDMCompPool"))
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Informations")
	FString Description;

	// 以项目文档为准
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Informations")
	FString OpenSourceLicense = TEXT("MPL2.0");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Informations")
	FString DocsURL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Informations")
	FString Developer;

	// 查找Helper组件的方式
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings",
			  meta = (UIMin = 0.001f, UIMax = 1.0f))
	EVFHelperGetting HelperGetting = EVFHelperGetting::ByVFHelperInterface;

	// 贴花光照强度修正
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings",
			  meta = (UIMin = 0.5f, UIMax = 2.0f))
	float PhotoDecalLightFix = 0.9f;

	// 使用GeometryScript插件或者内部已剥离的代码
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|Geometry")
	bool bUseGeometryScript = false;

	// TODO: 碰撞生成参数, 需要重启编辑器.

	// 仅bUseGeometryScript为false可用, 现版本法线存在问题.
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|Geometry|Frustum",
			  meta = (EditCondition = "!bUseGeometryScript"))
	bool bUseFrustumSegmented = true;

	// 视锥分段参数, 需要重启编辑器.
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|Geometry|Frustum",
			  meta = (EditCondition =
						  "!bUseGeometryScript && bUseFrustumSegmented"))
	FVector FrustumSegmentSize = FVector(2000.0f, 2000.0f, 2000.0f);

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

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|VFDMCompPool")
	bool bUseVFDMCompPool = true;

	// 组件对象池预生成数量
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|VFDMCompPool",
			  meta = (EditCondition = "bUseVFDMCompPool"))
	TMap<TSubclassOf<UVFDynamicMeshComponent>, int> CompPoolPrepareNum;
};