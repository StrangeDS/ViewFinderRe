// Copyright StangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VFPhotoContainerInterface.h"
#include "VFStepsRecordInterface.h"
#include "VFHelperInterface.h"
#include "VFStepsRecorderWorldSubsystem.h"
#include "VFCharacter.generated.h"

class UCameraComponent;
class APlayerController;
class UInputAction;
class UInputComponent;
class UInputMappingContext;

class AVFPhotoContainer_Input;
class UVFHelperComponent;
class IVFInteractInterface;
class IVFActivatableInterface;

USTRUCT(BlueprintType)
struct FVFPawnTransformInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	FRotator Rotator = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float Time = 0.f;

	FVFPawnTransformInfo() {};

	FVFPawnTransformInfo(APawn *Pawn, float TimeIn);

	FVFPawnTransformInfo(const FVFPawnTransformInfo &Other, float TimeIn);

	bool operator==(const FVFPawnTransformInfo &Other) const;
};

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERRE_API AVFCharacter : public ACharacter,
									  public IVFStepsRecordInterface,
									  public IVFPhotoContainerInterface,
									  public IVFHelperInterface
{
	GENERATED_BODY()

public:
	AVFCharacter();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) override;

	virtual void PossessedBy(AController *NewController) override;

	virtual void UnPossessed() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UVFHelperComponent> Helper;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TSubclassOf<AVFPhotoContainer_Input> ContainerClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<AVFPhotoContainer_Input> Container;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<APlayerController> PlayerController;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void TryToTraceInteractable(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void TraceInteractable();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float TraceInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float TraceDistance = 150.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "ViewFinder")
	float TimeSinceLastTrace = 0.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "ViewFinder")
	TScriptInterface<IVFInteractInterface> InteractingObject;

protected:
	void Move(const struct FInputActionValue &Value);

	void Look(const struct FInputActionValue &Value);

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void Interact();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UInputMappingContext> MappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UInputAction> SwitchAction;

public:
	virtual void TickForward_Implementation(float Time) override;

	virtual void TickBackward_Implementation(float Time) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TArray<FVFPawnTransformInfo> Steps;

	DECLARE_STEPSRECORDER_SUBSYSTEM_ACCESSOR(StepsRecorder);

public: // IVFPhotoContainerInterface
	virtual int GetPhoto2DNum_Implementation() override;

	virtual bool TakeIn_Implementation(AVFPhoto2D *Photo2D, const bool &Enabled) override;

public: // IVFHelperInterface
	virtual UVFHelperComponent *GetHelper_Implementation() override;

public: // Manage equipments of UVFActivatableInterface
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void SwitchEquipmentNext();
	virtual void SwitchEquipmentNext_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void AddEquipment(const TScriptInterface<IVFActivatableInterface> &Equipment);
	virtual void AddEquipment_Implementation(const TScriptInterface<IVFActivatableInterface> &Equipment);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void RemoveEquipment(const TScriptInterface<IVFActivatableInterface> &Equipment);
	virtual void RemoveEquipment_Implementation(const TScriptInterface<IVFActivatableInterface> &Equipment);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void DeactivateCurEquipment();
	virtual void DeactivateCurEquipment_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	bool SwitchEquipment(const TScriptInterface<IVFActivatableInterface> &Equipment);
	virtual bool SwitchEquipment_Implementation(const TScriptInterface<IVFActivatableInterface> &Equipment);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void OnEndRewinding(float Time);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	int EquipmentCurIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TArray<TScriptInterface<IVFActivatableInterface>> Equipments;
};