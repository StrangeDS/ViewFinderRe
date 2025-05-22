#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VFDMCompPoolWorldSubsystem.generated.h"

class UVFDynamicMeshComponent;

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API UVFDMCompPoolWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	UVFDMCompPoolWorldSubsystem();

public:
	virtual void Deinitialize() override;

public:
	// 获取组件
	UFUNCTION(BlueprintCallable, Category = "ViewFinder | DMCompPool")
	UVFDynamicMeshComponent *GetOrCreateComp(
		UObject *Outer,
		const TSubclassOf<UVFDynamicMeshComponent> &CompClass);

	// 归还组件
	UFUNCTION(BlueprintCallable, Category = "ViewFinder | DMCompPool")
	void ReturnComp(UVFDynamicMeshComponent *Comp);

	// 清理地址
	UFUNCTION(BlueprintCallable, Category = "ViewFinder | DMCompPool")
	void ClearComps(bool bForceGarbage = true);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder | DMCompPool")
	TArray<TObjectPtr<UVFDynamicMeshComponent>> AvailableComps;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder | DMCompPool")
	TArray<TObjectPtr<UVFDynamicMeshComponent>> AllComps;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder | DMCompPool")
	int SizeOfPool = 10000;
};
