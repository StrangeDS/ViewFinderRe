// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFHelperInterface.h"
#include "VFPlaneActor.generated.h"

#if WITH_EDITOR
class UMaterialInstanceDynamic;
#endif

// Draw the background to the plane of the AVFPlaneActor.
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