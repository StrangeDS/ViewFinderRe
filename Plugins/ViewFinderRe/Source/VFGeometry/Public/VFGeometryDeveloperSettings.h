// Copyright StrangeDS. All Rights Reserved.

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
	uint8 MinValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	uint8 MaxValue = 255;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	FVF_GeometryScriptCollisionFromMeshOptions Option;

	bool Contains(uint8 Value) const
	{
		return Value >= MinValue && Value <= MaxValue;
	}
};

// Including GeometryStrategy/ComponentsPool/CollisionParameters settings
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
	// Invalid value or default: UVFGeometryScriptNativeStrategy
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|GeometryStrategy",
			  meta = (MustImplement =
						  "/Script/VFGeometryBase.VFGeometryStrategyInterface"))
	TSubclassOf<UObject> GeometryStrategyClass;

	UFUNCTION(BlueprintCallable, Category = "Settings|GeometryStrategy")
	bool IsGeometryStrategyNone() const;

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|CompsPool")
	bool bUseVFDMCompsPool = true;

	/*
	Specify the numbers of components pre-generated in the pool.
	Due to the dependency of the module, it's better to set in levelscript.
	*/
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|CompsPool",
			  meta = (EditCondition = "bUseVFDMCompsPool"))
	TMap<TSubclassOf<UVFDynamicMeshComponent>, int> CompPoolPrepareNum;

public: // Mesh bool operation parameters
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|Boolean")
	FVF_GeometryScriptMeshBooleanOptions IntersectOption;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|Boolean")
	FVF_GeometryScriptMeshBooleanOptions SubtractOption;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|Boolean")
	FVF_GeometryScriptMeshBooleanOptions UnionOption;

public:
	// Parameters from (Static)MeshComponent to DynamicMeshComponent
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|DynamicMesh")
	FVF_GeometryScriptCopyMeshFromComponentOptions CopyMeshOption;

	/*
	Parameter array for generating simple collision.
	match sequentially, with earlier entries having higher priority.
	*/
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|DynamicMesh")
	TArray<FVFLevelOfCollisionMapping> CollisionLevels;

	UFUNCTION(BlueprintCallable, Category = "Settings|GeometryStrategy")
	const FVF_GeometryScriptCollisionFromMeshOptions GetCollisionOption(int Level) const;

public: // Separate parameters for frustum.(for performance)
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|ViewFrustum")
	FVF_GeometryScriptPrimitiveOptions ViewFrustumPrimitiveOption;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,
			  Category = "Settings|ViewFrustum")
	FVF_GeometryScriptCollisionFromMeshOptions ViewFrustumCollisionOption;
};