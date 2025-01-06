#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Containers/Deque.h"
#include "TimerManager.h"

#include "VFPhotoContainer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVFPhotoContainerEnabled, bool, Enabled);

class AVFPhoto2D;

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPhotoContainer : public AActor
{
	GENERATED_BODY()

public:
	AVFPhotoContainer();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	virtual void AddAPhoto(AVFPhoto2D *Photo);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	virtual void PrepareCurrentPhoto(float Time = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	virtual void GiveUpPreparing();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	virtual void PlaceCurrentPhoto();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	virtual void ChangeCurrentPhoto(const bool Next);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void UpdateCurrentPhoto();
	
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	virtual void SetEnabled(const bool &Enabled);
	
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	FORCEINLINE void SetPlayerController(APlayerController *Controller) { PlayerController = Controller; };

	UFUNCTION(BlueprintPure, Category = "ViewFinder")
	FORCEINLINE int Num() { return Photo2Ds.Num(); };
	
	UFUNCTION(BlueprintPure, Category = "ViewFinder")
	FORCEINLINE bool IsEnabled() { return bEnabled; };

public:
	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFPhotoContainerEnabled OnEnabled;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bFocusOn = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bEnabled = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float TimeOfSelect = 0.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<AVFPhoto2D> CurrentPhoto2D;

	// UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder") // 不能修饰
	TDeque<AVFPhoto2D *> Photo2Ds;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	FTimerHandle PrepareTimeHandle;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<USceneComponent> Container;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UVFHelperComponent> Helper;
};
