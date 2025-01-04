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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UVFPhotoCaptureComponent> PhotoCapture;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TArray<AActor *> ManagedActors;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UDecalComponent> Decal;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> Matirial;
	
	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFPhotoDecalDelegate OnReplace;
	
	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFPhotoDecalDelegate OnRestore;
};
