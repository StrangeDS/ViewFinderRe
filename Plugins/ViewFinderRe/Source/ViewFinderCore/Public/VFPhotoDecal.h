#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFPhotoDecal.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVFPhotoDecalDelegate);

class UTexture2D;
class UDecalComponent;
class UMaterialInstanceDynamic;

class UVFViewFrustumComponent;
class UVFPhotoCaptureComponent;

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPhotoDecal : public AActor
{
	GENERATED_BODY()

public:
	AVFPhotoDecal();

	virtual void OnConstruction(const FTransform &Transform) override;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void DrawDecal(bool ForceToUpdate = false);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void DrawSceneDepth(bool ForceToUpdate = false);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, CallInEditor, Category = "ViewFinder")
	void ReplaceWithDecal();
	virtual void ReplaceWithDecal_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, CallInEditor, Category = "ViewFinder")
	void RestoreWithActors();
	virtual void RestoreWithActors_Implementation();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void SetDecalEnabled(bool Enabled);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	void SetManagedActorsEnabled(bool Enabled);

#if WITH_EDITOR
	// 你可以用视锥收集后, 再手动删除不需要的Actor. 减少工作量.
	UFUNCTION(CallInEditor, Category = "ViewFinder")
	void RecollectActorsWithFrustum();
#endif

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<USceneComponent> CaptureRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UVFPhotoCaptureComponent> CaptureOfDecal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UVFPhotoCaptureComponent> CaptureOfDepth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UVFViewFrustumComponent> ViewFrustum;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UDecalComponent> Decal;

public:
	// ViewAngle应当尽可能的小, 贴花才更清晰
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float ViewAngle = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float StartDis = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float EndDis = 1000.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float AspectRatio = 1.666667f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TArray<AActor *> ManagedActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bOnlyCatchManagedActors = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bReplacing = false;

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	UMaterialInstanceDynamic *GetMaterialInstance();
	virtual UMaterialInstanceDynamic* GetMaterialInstance_Implementation();

	UPROPERTY()
	TObjectPtr<UMaterialInterface> Matirial;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UMaterialInstanceDynamic> MaterialInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UTexture2D> TextureOfDecal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UTexture2D> TextureOfDepth;

public:
	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFPhotoDecalDelegate OnReplace;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFPhotoDecalDelegate OnRestore;
};
