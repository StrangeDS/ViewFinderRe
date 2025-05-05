#pragma once

#include "CoreMinimal.h"
#include "VFPawnStandIn.h"
#include "VFStepsRecordInterface.h"
#include "VFPawnStandInSteppable.generated.h"

class UVFStepsRecorderWorldSubsystem;

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPawnStandInSteppable : public AVFPawnStandIn, public IVFStepsRecordInterface
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

public:
	virtual bool StepBack_Implementation(FVFStepInfo &StepInfo) override;

	UPROPERTY(BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UVFStepsRecorderWorldSubsystem> StepRecorder;
};
