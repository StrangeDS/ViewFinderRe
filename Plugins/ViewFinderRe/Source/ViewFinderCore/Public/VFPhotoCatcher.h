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
class VIEWFINDERCORE_API AVFPhotoCatcher : public AActor, public IVFHelperInterface
{
	GENERATED_BODY()

public:
	AVFPhotoCatcher();

	virtual void OnConstruction(const FTransform &Transform) override;

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

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void ResetActorsToIgnore();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void EnableScreen(const bool &Enabled = true);

	UFUNCTION(BlueprintPure, Category = "ViewFinder")
	FQuat GetFrustumQuat();

	UFUNCTION(BlueprintPure, Category = "ViewFinder")
	FORCEINLINE bool IsCuttingOrignal() { return bCuttingOrignal; };

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float ViewAngle = 60.0f;

	// 宽高比, 但设置了渲染目标后会以渲染目标为准.
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

	// 只与携带了Helper组件的Actor进行交互. 对于插入已有场景来说会很方便.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	bool bOnlyOverlapWithHelps = false;

	// 拍照是否切割原物体. 开启后则不是复制, 而是剪切.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	bool bCuttingOrignal = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TObjectPtr<AActor>> ActorsToIgnore;

	// 最好指定层级. 不指定将会overlap所有物体类型, 包括角色本身.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesToOverlap;

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
};