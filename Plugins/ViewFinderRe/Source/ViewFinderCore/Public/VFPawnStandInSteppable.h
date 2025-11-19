// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VFPawnStandIn.h"
#include "VFStepsRecordInterface.h"
#include "VFPawnStandInSteppable.generated.h"

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPawnStandInSteppable : public AVFPawnStandIn, public IVFStepsRecordInterface
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

public:
	virtual bool StepBack_Implementation(FVFStepInfo &StepInfo) override;
};