#pragma once

#include "CoreMinimal.h"
#include "VFDeveloperSettingsCommon.h"
#include "VFGeometryHeaders.h"
#include "Components/DynamicMeshComponent.h"
#include "VFGeometryDeveloperSettings.generated.h"

class UVFDynamicMeshComponent;

USTRUCT(BlueprintType)
struct FVFLevelOfCollisionMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	uint8 MinValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	uint8 MaxValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	FVF_GeometryScriptCollisionFromMeshOptions Option;

	bool Contains(uint8 Value) const
	{
		return Value >= MinValue && Value <= MaxValue;
	}
};

// 包括几何策略, 组件对象池, 生成碰撞的参数设置
UCLASS(Config = ViewFinderReSettings, defaultconfig,
	   autoExpandCategories =
		   ("Settings", "Settings|GeometryStrategy",
			"Settings|CompsPool", "Settings|Boolean",
			"Settings|DynamicMesh", "Settings|ViewFrustum"))
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
	bool IsGeometryStrategyNone() const;

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

public: // 布尔操作参数
	// 网格交集参数
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|Boolean")
	FVF_GeometryScriptMeshBooleanOptions IntersectOption;

	// 网格差集参数
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|Boolean")
	FVF_GeometryScriptMeshBooleanOptions SubtractOption;

	// 网格并集参数
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|Boolean")
	FVF_GeometryScriptMeshBooleanOptions UnionOption;

public:
	// 从(静态)网格复制到动态网格的参数
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|DynamicMesh")
	FVF_GeometryScriptCopyMeshFromComponentOptions CopyMeshOption;

	// 生产简单碰撞的参数, 依次匹配, 在前的优先级更高
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|DynamicMesh")
	TArray<FVFLevelOfCollisionMapping> CollisionLevels;

	UFUNCTION(BlueprintCallable, Category = "Settings|GeometryStrategy")
	const FVF_GeometryScriptCollisionFromMeshOptions GetCollisionOption(int Level) const;

public: // 视锥需要单独处理碰撞参数(省性能)
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|ViewFrustum")
	FVF_GeometryScriptPrimitiveOptions ViewFrustumPrimitiveOption;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|ViewFrustum")
	FVF_GeometryScriptCollisionFromMeshOptions ViewFrustumCollisionOption;
};