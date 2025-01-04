#pragma once

#include "CoreMinimal.h"
#include "VFPhotoContainer.h"

#include "VFStepsRecordInterface.h"
#include "VFStepsRecorderWorldSubsystem.h"

#include "VFPhotoContainerSteppable.generated.h"

UENUM(BlueprintType)
enum class AVFPhotoContainerSteppableOperation : uint8
{
	None = 0,
	Add,
	Place,
	Prepare,
	GiveUpPreparing,
	ChangeNext,
	ChangeLast,
	Enabled,
	Disabled
};

USTRUCT(BlueprintType)
struct FVFPhotoContainerStepInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	AVFPhotoContainerSteppableOperation Operation = AVFPhotoContainerSteppableOperation::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<class AVFPhoto2D> Photo;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float Time = 0.f;
};

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPhotoContainerSteppable : public AVFPhotoContainer, public IVFStepsRecordInterface
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	virtual void AddAPhoto(AVFPhoto2D *Photo) override;

	virtual void PlaceCurrentPhoto() override;

	virtual void PrepareCurrentPhoto(float Time = -1.0f) override;

	virtual void GiveUpPreparing() override;

	virtual void ChangeCurrentPhoto(const bool Next) override;
	
	virtual void SetEnabled(const bool &Enabled);

public: // Implements IVFStepsRecordInterface:
	virtual void TickBackward_Implementation(float Time) override;
	
	virtual bool StepBack_Implementation(FVFStepInfo &StepInfo) override;

	TObjectPtr<UVFStepsRecorderWorldSubsystem> StepRecorder;

	UPROPERTY(BlueprintReadOnly, Category = "ViewFinder")
	TArray<FVFPhotoContainerStepInfo> Steps;
};
