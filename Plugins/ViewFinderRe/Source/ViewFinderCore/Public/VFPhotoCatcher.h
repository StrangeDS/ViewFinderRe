#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFHelperInterface.h"
#include "VFPhotoCatcher.generated.h"

class UStaticMeshComponent;

class AVFPhoto2D;
class AVFPhoto3D;
class UVFHelperComponent;
class UVFViewFrustumComponent;
class UVFDynamicMeshComponent;
class UVFPhotoCaptureComponent;

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPhotoCatcher : public AActor,  public IVFHelperInterface
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	bool bOnlyOverlapWithHelps = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	bool bCuttingOrignal = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TObjectPtr<AActor>> ActorsToIgnore;

	// 最好指定层级. 不指定将会overlap所有物体类型, 包括角色本身.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesToOverlap;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CatcherMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UVFPhotoCaptureComponent> PhotoCapture;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UVFViewFrustumComponent> ViewFrustum;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UVFHelperComponent> Helper;

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	UMaterialInstanceDynamic *GetScreenMID();
	virtual UMaterialInstanceDynamic* GetScreenMID_Implementation();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UMaterialInstanceDynamic> ScreenMID;

public: // IVFHelperInterface
	virtual UVFHelperComponent *GetHelper_Implementation() override;
};