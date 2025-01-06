#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"

#include "VFPhotoCatcher.generated.h"

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPhotoCatcher : public AActor
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
	AVFPhoto2D *TakeAPhoto();
	virtual AVFPhoto2D *TakeAPhoto_Implementation();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void SetViewFrustumVisible(const bool &Visibility);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void ResetActorsToIgnore();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float ViewAngle = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float AspectRatio = 16.0f / 9;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float StartDis = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float EndDis = 20000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder|ClassSetting")
	TSubclassOf<class UVFDynamicMeshComponent> VFDMCompClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder|ClassSetting")
	TSubclassOf<class AVFPhoto2D> VFPhoto2DClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder|ClassSetting")
	TSubclassOf<class AVFPhoto3D> VFPhoto3DClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder|ClassSetting")
	TSubclassOf<class AVFPawnStandIn> VFPawnStandInClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	bool bOnlyOverlapWithHelps = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	bool bCuttingOrignal = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TObjectPtr<AActor>> ActorsToIgnore;

	// 最好指定层级. 不指定将会overlap所有物体类型, 包括角色本身.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesToOverlap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UStaticMeshComponent> StaticMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CatcherMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UVFPhotoCaptureComponent> PhotoCapture;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UVFViewFrustumComponent> ViewFrustum;
};
