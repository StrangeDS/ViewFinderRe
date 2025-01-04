#include "VFPhotoCatcherSteppable.h"

void AVFPhotoCatcherSteppable::BeginPlay()
{
    Super::BeginPlay();

    StepRecorder = GetWorld()->GetSubsystem<UVFStepsRecorderWorldSubsystem>();
    check(StepRecorder);
}

AVFPhoto2D *AVFPhotoCatcherSteppable::TakeAPhoto_Implementation()
{
    StepRecorder->SubmitStep(this, FVFStepInfo{TEXT("Take A Photo"), true});

    return Super::TakeAPhoto_Implementation();
}

bool AVFPhotoCatcherSteppable::StepBack_Implementation(FVFStepInfo &StepInfo)
{
    // Nothing To Do.
    return true;
}
