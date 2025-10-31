#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFHelperInterface.h"
#include "VFPhotoDecal.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVFPhotoDecalDelegate);

class UDecalComponent;
class UMaterialInstanceDynamic;

class UVFViewFrustumComponent;
class UVFPhotoCaptureComponent;

USTRUCT(BlueprintType)
struct FVFPhotoDecalRecordProps
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bSimulatePhysics = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bGravity = false;
};

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VFPHOTODECAL_API AVFPhotoDecal : public AActor,
									   public IVFHelperInterface
{
	GENERATED_BODY()

public:
	AVFPhotoDecal();

	virtual void OnConstruction(const FTransform &Transform) override;

public:
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "ViewFinder")
	void UpdateMaterialParams();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void DrawDecal();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void DrawSceneDepth();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, CallInEditor, Category = "ViewFinder")
	void ReplaceWithDecal();
	virtual void ReplaceWithDecal_Implementation();

#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	void ReplaceWithDecalInEditor();
#endif

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UVFHelperComponent> Helper;

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

	// 使用此项将使用ShowOnlyActors, 注意: 将不会接收到其它物体的遮挡和阴影效果
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bOnlyCatchManagedActors = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bReplacing = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TMap<UPrimitiveComponent *, FVFPhotoDecalRecordProps> PropsMap;

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	UMaterialInstanceDynamic *GetMaterialInstance();
	virtual UMaterialInstanceDynamic *GetMaterialInstance_Implementation();

	UPROPERTY()
	TObjectPtr<UMaterialInterface> Matirial;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UMaterialInstanceDynamic> MaterialInstance;

public:
	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFPhotoDecalDelegate OnReplace;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFPhotoDecalDelegate OnRestore;

public: // IVFHelperInterface
	virtual UVFHelperComponent *GetHelper_Implementation() override;
};
