#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFHelperInterface.h"
#include "VFPlaneActor.generated.h"

#if WITH_EDITOR
class UMaterialInstanceDynamic;
#endif

/*
背景映射在AVFPlaneActor的Plane上
请注意, 由于视锥分段, 导致布尔得到的面布线很烂, 生成的碰撞是没有的.
*/
UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VFPHOTOCATCHER_API AVFPlaneActor : public AActor,
										 public IVFHelperInterface
{
	GENERATED_BODY()

public:
	AVFPlaneActor();

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void SetPlane(FVector Location, FVector Direction, float Width, float Height);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void SetPlaneMaterial(UTexture2D *Texture);

#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	void FaceToPawn();
#endif

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<USceneComponent> TransformRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UStaticMeshComponent> Plane;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UVFHelperComponent> Helper;

public: // IVFHelperInterface
	virtual UVFHelperComponent *GetHelper_Implementation() override;

#if WITH_EDITOR
public:
	UMaterialInstanceDynamic *GetMaterialInstanceInEditor();
#endif
};