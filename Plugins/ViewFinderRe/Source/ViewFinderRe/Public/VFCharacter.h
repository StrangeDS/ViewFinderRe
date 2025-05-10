#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VFPhoto2DContainerInterface.h"
#include "VFStepsRecordInterface.h"
#include "VFCharacter.generated.h"

class UCameraComponent;
class APlayerController;
class UInputAction;
class UInputComponent;
class UInputMappingContext;

class AVFPhotoContainer;
class UVFHelperComponent;
class IVFInteractInterface;
class UVFStepsRecorderWorldSubsystem;

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
class VIEWFINDERRE_API AVFCharacter : public ACharacter, public IVFStepsRecordInterface, public IVFPhoto2DContainerInterface
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UVFHelperComponent> Helper;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TSubclassOf<AVFPhotoContainer> ContainerClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<AVFPhotoContainer> Container;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "ViewFinder")
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

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void Switch();

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

	TObjectPtr<UVFStepsRecorderWorldSubsystem> StepRecorder;

public: // IVFPhoto2DContainerInterface
	virtual int GetPhoto2DNum_Implementation() override;

	virtual bool TakeIn_Implementation(AVFPhoto2D *Photo2D, const bool &Enabled) override;
};