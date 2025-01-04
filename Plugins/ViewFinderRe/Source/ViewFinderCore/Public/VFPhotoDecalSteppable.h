#pragma once

#include "CoreMinimal.h"
#include "VFPhotoDecal.h"

#include "VFStepsRecordInterface.h"
#include "VFStepsRecorderWorldSubsystem.h"

#include "VFPhotoDecalSteppable.generated.h"

UENUM(BlueprintType)
enum class AVFPhotoDecalOperation : uint8
{
	None = 0,
	Replace,
	Restore
};


UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPhotoDecalSteppable : public AVFPhotoDecal, public IVFStepsRecordInterface
{
	GENERATED_BODY()

public:	
	virtual void BeginPlay() override;

	virtual void Replace() override;
	
	virtual void Restore() override;

public:
	virtual bool StepBack_Implementation(FVFStepInfo &StepInfo) override;
	
	TObjectPtr<UVFStepsRecorderWorldSubsystem> StepRecorder;
};