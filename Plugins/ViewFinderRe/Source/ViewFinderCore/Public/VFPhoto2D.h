#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFPhoto2D.generated.h"

class UTexture2D;
class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

class AVFPhoto3D;
class UVFHelperComponent;
class UVFPhotoCaptureComponent;

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

	virtual void BeginPlay() override;

	virtual void SetActorHiddenInGame(bool bNewHidden) override;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void SetPhoto3D(AVFPhoto3D *Photo);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
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

protected:	// 组件
	UPROPERTY()
	TObjectPtr<UStaticMesh> StaticMeshObject;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UVFHelperComponent> Helper;

protected:	// 状态, 数据
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	EVFPhoto2DState State = EVFPhoto2DState::None;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<AVFPhoto3D> Photo3D;

public: // 动态材质实例相关
	// 懒加载, 也便于重写
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	UMaterialInstanceDynamic *GetMaterialInstance();
	virtual UMaterialInstanceDynamic* GetMaterialInstance_Implementation();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	FName TextureName = TEXT("Texture");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	FName RatioName = TEXT("AspectRatio");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UMaterialInstanceDynamic> MaterialInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UTexture2D> Texture2D;
};
