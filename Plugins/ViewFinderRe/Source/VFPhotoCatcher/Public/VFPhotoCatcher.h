// Copyright 2026, StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFHelperInterface.h"
#include "VFPhotoCatcher.generated.h"

class UStaticMeshComponent;
class AVFPhoto2D;
class AVFPhoto3D;
class UVFViewFrustumComponent;
class UVFDynamicMeshComponent;
class UVFPhotoCaptureComponent;
class UVFBackgroundCaptureComponent;
class UVFPostProcessComponent;

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VFPHOTOCATCHER_API AVFPhotoCatcher : public AActor, public IVFHelperInterface
{
	GENERATED_BODY()

public:
	AVFPhotoCatcher();

	virtual void OnConstruction(const FTransform &Transform) override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext &Context) const override;
#endif

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	TArray<UPrimitiveComponent *> GetOverlapComps();
	virtual TArray<UPrimitiveComponent *> GetOverlapComps_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	TArray<UPrimitiveComponent *> FilterOverlapComps(
		const TArray<UPrimitiveComponent *> &Comps);
	virtual TArray<UPrimitiveComponent *> FilterOverlapComps_Implementation(
		const TArray<UPrimitiveComponent *> &Comps);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	AVFPhoto2D *TakeAPhoto();
	virtual AVFPhoto2D *TakeAPhoto_Implementation();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void SetViewFrustumVisible(const bool &Visibility);

#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	void ResetActorsToIgnore();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	void DrawABackgroundPlaneInEditor();
#endif

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void EnableScreen(const bool &Enabled = true);

	UFUNCTION(BlueprintPure, Category = "ViewFinder")
	FQuat GetFrustumQuat();

	UFUNCTION(BlueprintPure, Category = "ViewFinder")
	FORCEINLINE bool IsCuttingOriginal() { return bCuttingOriginal; };

	UFUNCTION(BlueprintPure, Category = "ViewFinder")
	bool HasAnyLens();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void PostProcessComps(TArray<UPrimitiveComponent *> Comps);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float ViewAngle = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float AspectRatio = 16.0f / 9;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float StartDis = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float EndDis = 20000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder|ClassSetting")
	TSubclassOf<UVFDynamicMeshComponent> VFDMCompClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder|ClassSetting")
	TSubclassOf<AVFPhoto2D> VFPhoto2DClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder|ClassSetting")
	TSubclassOf<AVFPhoto3D> VFPhoto3DClass;

	/*
	Only process Actors that have Helper component.
	It's convenient for using in existing scenes.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	bool bOnlyOverlapWithHelper = false;

	/*
	Whether to cut the original object when taking a photo.
	When enabled, it's not copying but cutting.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	bool bCuttingOriginal = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TObjectPtr<AActor>> ActorsToIgnore;

	// If not specified, it will overlap all object types. For future use.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesToOverlap;

	// Works together with UVFPhotoCatcherDeveloperSettings::bGeneratePlaneActor
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	bool bGenerateAPlaneActor = true;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CatcherMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UVFPhotoCaptureComponent> PhotoCapture;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UVFBackgroundCaptureComponent> BackgroundCapture;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UVFViewFrustumComponent> ViewFrustum;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UVFHelperComponent> Helper;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UVFPostProcessComponent> PostProcess;

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	UMaterialInstanceDynamic *GetScreenMID();
	virtual UMaterialInstanceDynamic *GetScreenMID_Implementation();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UMaterialInstanceDynamic> ScreenMID;

public: // IVFHelperInterface
	virtual UVFHelperComponent *GetHelper_Implementation() override;

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void HandleOriginalBeforeCheckVFDMComps(UObject *Sender);
	void HandleOriginalBeforeCheckVFDMComps_Implementation(UObject *Sender);

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void HandleCopyEndPlacingPhoto(UObject *Sender);
	void HandleCopyEndPlacingPhoto_Implementation(UObject *Sender);
};