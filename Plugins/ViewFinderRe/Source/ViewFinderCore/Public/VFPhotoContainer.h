#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFPhotoContainerInterface.h"
#include "Containers/Deque.h"
#include "VFPhotoContainer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVFPhotoContainerEnabled, bool, Enabled);

class AVFPhoto2D;

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPhotoContainer : public AActor, public IVFPhotoContainerInterface
{
	GENERATED_BODY()

public:
	AVFPhotoContainer();

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	virtual void AddAPhoto(AVFPhoto2D *Photo);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	virtual void PrepareCurrentPhoto();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	virtual void GiveUpPreparing();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	virtual void PlaceCurrentPhoto();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	virtual void ChangeCurrentPhoto(const bool Next);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void UpdateCurrentPhoto();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void RotateCurrentPhoto(float Delta);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void AlignCurrentPhoto();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	virtual void SetEnabled(const bool &EnabledIn);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	FORCEINLINE void SetPlayerController(APlayerController *Controller) { PlayerController = Controller; };

	UFUNCTION(BlueprintPure, Category = "ViewFinder")
	FORCEINLINE int Num() { return Photo2Ds.Num(); };

	UFUNCTION(BlueprintPure, Category = "ViewFinder")
	FORCEINLINE bool IsEnabled() { return bEnabled; };

public:
	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFPhotoContainerEnabled OnEnabled;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bFocusOn = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bEnabled = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<AVFPhoto2D> CurrentPhoto2D;

	// UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder") // 不能修饰
	TDeque<AVFPhoto2D *> Photo2Ds;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void PrepareCurrentPhoto_Move();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void GiveUpPreparing_Move();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	FTimerHandle PrepareTimeHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float TimeOfPrepare = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float TimeOfGivingUp = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float RotateFactor = -2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float PrepareMoveInterval = 0.02f;

protected:
	// Photo2D的实际位置
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<USceneComponent> Container;

	// 预览Photo2D的位置
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<USceneComponent> Preview;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<class UVFHelperComponent> Helper;

public: // IVFPhotoContainerInterface
	virtual int GetPhoto2DNum_Implementation() override;

	virtual bool TakeIn_Implementation(AVFPhoto2D *Photo2D, const bool &Enabled) override;
};
