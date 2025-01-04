#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "UObject/ConstructorHelpers.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "VFPhoto2D.generated.h"

UENUM(BlueprintType)
enum class EVFPhoto2DState : uint8
{
	None,
	Folded,
	Placed
};

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPhoto2D : public AActor
{
	GENERATED_BODY()

public:
	AVFPhoto2D();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetActorHiddenInGame(bool bNewHidden) override;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void SetPhoto3D(class AVFPhoto3D *Photo);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	AVFPhoto3D *GetPhoto3D();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void SetPhoto(USceneCaptureComponent2D *Capturer);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	virtual void FoldUp();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	void Preview(const FTransform& WorldTrans, const bool &Enabled);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	virtual void PlaceDown();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void CopyPhoto3D();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	EVFPhoto2DState State = EVFPhoto2DState::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UStaticMeshComponent> StaticMesh;
	
	UPROPERTY()
	TObjectPtr<class UStaticMesh> StaticMeshObject;

public: // 材质相关
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	int PixelNum = 1024;
	
	UPROPERTY(VisibleAnywhere, Category = "ViewFinder")
	float AspectRatio = 16.0f / 9;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	FName TextureName = TEXT("Texture");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	FName RatioName = TEXT("AspectRatio");

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UMaterialInstanceDynamic> MaterialInstance;

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<AVFPhoto3D> Photo3D;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UVFHelperComponent> Helper;
};
