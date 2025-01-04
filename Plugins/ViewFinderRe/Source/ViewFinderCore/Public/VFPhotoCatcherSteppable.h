#pragma once

#include "CoreMinimal.h"
#include "VFPhotoCatcher.h"

#include "VFStepsRecordInterface.h"
#include "VFStepsRecorderWorldSubsystem.h"

#include "VFPhotoCatcherSteppable.generated.h"

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPhotoCatcherSteppable : public AVFPhotoCatcher, public IVFStepsRecordInterface
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	
	virtual AVFPhoto2D *TakeAPhoto_Implementation() override;

public:
	virtual bool StepBack_Implementation(FVFStepInfo &StepInfo) override;
	
	UPROPERTY(BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UVFStepsRecorderWorldSubsystem> StepRecorder;
};
