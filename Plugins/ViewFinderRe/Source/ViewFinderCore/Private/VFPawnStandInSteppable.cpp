#include "VFPawnStandInSteppable.h"

void AVFPawnStandInSteppable::BeginPlay()
{
    Super::BeginPlay();

    StepRecorder = GetWorld()->GetSubsystem<UVFStepsRecorderWorldSubsystem>();
    check(StepRecorder);
    StepRecorder->SubmitStep(this, FVFStepInfo{TEXT("PawnStandIn")});
}

bool AVFPawnStandInSteppable::StepBack_Implementation(FVFStepInfo &StepInfo)
{
    Destroy();
    return true;
}
