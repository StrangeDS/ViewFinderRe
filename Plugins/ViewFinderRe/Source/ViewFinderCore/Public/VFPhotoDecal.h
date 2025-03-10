#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "UObject/ConstructorHelpers.h"

#include "VFPhotoDecal.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVFPhotoDecalDelegate);

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPhotoDecal : public AActor
{
	GENERATED_BODY()

public:
	AVFPhotoDecal();

	virtual void OnConstruction(const FTransform &Transform) override;

	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void DrawDecal();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	virtual void Replace();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	virtual void Restore();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void SetDecalEnabled(bool Enabled);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	void SetManagedActorsEnabled(bool Enabled);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UVFPhotoCaptureComponent> PhotoCapture;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UVFViewFrustumComponent> ViewFrustum;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TArray<AActor *> ManagedActors;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UDecalComponent> Decal;

public:
	// ViewAngle应当尽可能的小, 贴花才更清晰
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float ViewAngle = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float AspectRatio = 16.0f / 9;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float StartDis = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float EndDis = 1000.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	bool bReplacing = false;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> Matirial;

	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> MaterialInstance;
	
	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFPhotoDecalDelegate OnReplace;
	
	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFPhotoDecalDelegate OnRestore;
};
