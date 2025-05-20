#include "VFPawnStandInSteppable.h"

#include "VFStepsRecorderWorldSubsystem.h"

void AVFPawnStandInSteppable::BeginPlay()
{
    Super::BeginPlay();

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        StepsRecorder->SubmitStep(this, FVFStepInfo{TEXT("PawnStandIn")});
    }
}

bool AVFPawnStandInSteppable::StepBack_Implementation(FVFStepInfo &StepInfo)
{
    Destroy();
    return true;
}
