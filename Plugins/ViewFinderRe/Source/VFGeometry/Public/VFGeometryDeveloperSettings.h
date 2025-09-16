#pragma once

#include "CoreMinimal.h"
#include "VFDeveloperSettingsCommon.h"
#include "Components/DynamicMeshComponent.h"
#include "VFGeometryDeveloperSettings.generated.h"

class UVFDynamicMeshComponent;

// 包括几何策略, 组件对象池, 生成碰撞的参数设置
UCLASS(Config = ViewFinderReSettings, defaultconfig,
	   autoExpandCategories =
		   ("Settings", "Settings|GeometryStrategy", "Settings|CompsPool"))
class VFGEOMETRY_API UVFGeometryDeveloperSettings : public UVFDeveloperSettingsCommon
{
	GENERATED_BODY()

public:
	UVFGeometryDeveloperSettings(
		const FObjectInitializer &ObjectInitializer = FObjectInitializer::Get());

#if WITH_EDITOR
	virtual void PostEditChangeProperty(
		struct FPropertyChangedEvent &PropertyChangedEvent) override;
#endif

public:
	// 空值会默认使用UVFGeometryScriptNativeStrategy
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|GeometryStrategy",
			  meta = (MustImplement =
						  "/Script/VFGeometryBase.VFGeometryStrategyInterface"))
	TSubclassOf<UObject> GeometryStrategyClass;

	UFUNCTION(BlueprintCallable, Category = "Settings|GeometryStrategy")
	static bool IsGeometryStrategyNone();

public:
	// 使用对象池
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|CompsPool")
	bool bUseVFDMCompsPool = true;

	// 组件对象池预生成数量
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|CompsPool",
			  meta = (EditCondition = "bUseVFDMCompsPool"))
	TMap<TSubclassOf<UVFDynamicMeshComponent>, int> CompPoolPrepareNum;

public:
	// TODO: 碰撞生成参数, 需要重启编辑器.
};