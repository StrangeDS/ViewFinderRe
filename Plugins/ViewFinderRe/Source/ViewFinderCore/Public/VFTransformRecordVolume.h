#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFTransformRecordVolume.generated.h"

class UBoxComponent;
class UVFHelperComponent;

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFTransformRecordVolume : public AActor
{
	GENERATED_BODY()
public:
	AVFTransformRecordVolume(const FObjectInitializer &ObjectInitializer = FObjectInitializer::Get());
	
public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	TArray<UPrimitiveComponent *> GetComponents();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TSubclassOf<UPrimitiveComponent> CompClass;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UBoxComponent> Volume;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UVFHelperComponent> Helper;
};
