#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VFDMCompPoolWorldSubsystem.generated.h"

class UVFDynamicMeshComponent;

USTRUCT(BlueprintType)
struct FVFDMCompPool
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<UVFDynamicMeshComponent>> Comps;
};

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API UVFDMCompPoolWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	UVFDMCompPoolWorldSubsystem();

public:
	virtual void Initialize(FSubsystemCollectionBase &Collection) override;

	virtual void Deinitialize() override;

public:
	// 获取组件
	UFUNCTION(BlueprintCallable, Category = "ViewFinder | DMCompPool")
	UVFDynamicMeshComponent *GetOrCreateComp(
		UObject *Outer,
		const TSubclassOf<UVFDynamicMeshComponent> &CompClass);

	// 归还组件
	UFUNCTION(BlueprintCallable, Category = "ViewFinder | DMCompPool")
	bool ReturnComp(UVFDynamicMeshComponent *Comp);

	// 预生成池
	UFUNCTION(BlueprintCallable, Category = "ViewFinder | DMCompPool")
	void PreparePools();

	// 清理地址
	UFUNCTION(BlueprintCallable, Category = "ViewFinder | DMCompPool")
	void ClearPools(bool bForceGarbage = true);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder | DMCompPool")
	TMap<TSubclassOf<UVFDynamicMeshComponent>, FVFDMCompPool> PoolsOfAvailable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder | DMCompPool")
	TMap<TSubclassOf<UVFDynamicMeshComponent>, FVFDMCompPool> PoolsOfAll;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder | DMCompPool")
	TMap<TSubclassOf<UVFDynamicMeshComponent>, int> PrepareNum;
};