#pragma once

#include "CoreMinimal.h"
#include "VFPhoto2D.h"

#include "VFStepsRecordInterface.h"
#include "VFStepsRecorderWorldSubsystem.h"

#include "VFPhoto2DSteppable.generated.h"

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPhoto2DSteppable : public AVFPhoto2D, public IVFStepsRecordInterface
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	
	virtual void FoldUp() override;

	virtual void PlaceDown() override;

public:
	virtual bool StepBack_Implementation(FVFStepInfo &StepInfo) override;
	
	UPROPERTY(BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UVFStepsRecorderWorldSubsystem> StepRecorder;
};
