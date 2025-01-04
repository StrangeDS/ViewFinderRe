#pragma once

#include "CoreMinimal.h"
#include "VFPhoto3D.h"

#include "VFStepsRecordInterface.h"
#include "VFStepsRecorderWorldSubsystem.h"

#include "VFPhoto3DSteppable.generated.h"

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPhoto3DSteppable : public AVFPhoto3D, public IVFStepsRecordInterface
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
