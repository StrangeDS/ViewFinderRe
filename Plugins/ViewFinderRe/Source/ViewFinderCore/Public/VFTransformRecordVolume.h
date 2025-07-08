#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFHelperInterface.h"
#include "VFTransformRecordVolume.generated.h"

class UBoxComponent;
class UVFHelperComponent;

/*
编辑器中放置.
其囊括的, CompClass类型的组件会被自动注册到AVFTransfromRecorderActor.
若在Photo3D中放置, 需要:
添加到Photo3D的ActorsToIgnore, 设置CompClass为VFDynamicMeshComponent.
*/
UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFTransformRecordVolume : public AActor,
													public IVFHelperInterface
{
	GENERATED_BODY()
public:
	AVFTransformRecordVolume(const FObjectInitializer &ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	TArray<UPrimitiveComponent *> GetComponents();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TSubclassOf<UPrimitiveComponent> CompClass;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	bool bEnabled = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<class UBoxComponent> Volume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<class UVFHelperComponent> Helper;

public:
	virtual UVFHelperComponent *GetHelper_Implementation() override;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void HandleCopyEndPlacingPhoto(UObject *Sender);
};