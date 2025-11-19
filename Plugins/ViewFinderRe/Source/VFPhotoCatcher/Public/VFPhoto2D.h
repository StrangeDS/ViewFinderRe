// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFHelperInterface.h"
#include "VFPhoto2D.generated.h"

class UTexture2D;
class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

class AVFPhoto3D;
class UVFHelperComponent;
class UVFPhotoCaptureComponent;
class AVFPhotoCatcher;

UENUM(BlueprintType)
enum class EVFPhoto2DState : uint8
{
	None,
	Folded,
	Placed
};

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VFPHOTOCATCHER_API AVFPhoto2D : public AActor,
									  public IVFHelperInterface
{
	GENERATED_BODY()

public:
	AVFPhoto2D();

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void SetPhoto3D(AVFPhoto3D *Photo);

	UFUNCTION(BlueprintPure, Category = "ViewFinder")
	AVFPhoto3D *GetPhoto3D();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void SetPhoto(UVFPhotoCaptureComponent *PhotoCapture = nullptr);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	virtual void FoldUp();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	void Preview(const FTransform &WorldTrans, const bool &Enabled);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	virtual void PlaceDown();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void CopyPhoto3D(UObject *Sender);

	// Recursive post-processing for Photo2D
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static void PostProcessPhoto2D(AVFPhotoCatcher *Catcher, AVFPhoto2D *Photo2D,
								   bool Recursively = false);

protected: // Components
	UPROPERTY()
	TObjectPtr<UStaticMesh> StaticMeshObject;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UVFHelperComponent> Helper;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	EVFPhoto2DState State = EVFPhoto2DState::None;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<AVFPhoto3D> Photo3D;

public: // About dynamic material instance
	// Lazy initialization, also facilitates overrides
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	UMaterialInstanceDynamic *GetMaterialInstance();
	virtual UMaterialInstanceDynamic *GetMaterialInstance_Implementation();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	FName TextureName = TEXT("Texture");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	FName RatioName = TEXT("AspectRatio");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UMaterialInstanceDynamic> MaterialInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UTexture2D> Texture2D;

public:
	/// @brief Designed for easy subclass overrides (enabling logic insertion) to AttachToActor() and DetachFromActor()
	/// @param Target Nullptr means DetachFromActor
	/// @return A false return value indicates no changes were made
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	virtual bool ReattachToComponent(USceneComponent *Target = nullptr);

public: // Recursive-related
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void CopyRecursivePhoto3D(UObject *Sender);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	bool bIsRecursive = false;

public: // IVFHelperInterface
	virtual UVFHelperComponent *GetHelper_Implementation() override;

#if WITH_EDITOR
public:
	UStaticMeshComponent *GetStaticMeshInEditor() { return StaticMesh; };
#endif
};