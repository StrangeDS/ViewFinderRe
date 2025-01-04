#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// 插件打包需要
#include "Components/StaticMeshComponent.h"

#include "VFTransformRecordVolume.generated.h"

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFTransformRecordVolume : public AActor
{
	GENERATED_BODY()
public:
	AVFTransformRecordVolume(const FObjectInitializer &ObjectInitializer);
	
public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	TArray<UPrimitiveComponent *> GetComponents();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TSubclassOf<UPrimitiveComponent> CompClass = UStaticMeshComponent::StaticClass();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UBoxComponent> Volume;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UVFHelperComponent> Helper;
};
