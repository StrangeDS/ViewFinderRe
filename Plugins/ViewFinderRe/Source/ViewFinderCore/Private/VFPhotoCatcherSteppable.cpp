#include "VFPhotoCatcherSteppable.h"

#include "VFStepsRecorderWorldSubsystem.h"

void AVFPhotoCatcherSteppable::BeginPlay()
{
    Super::BeginPlay();
}

AVFPhoto2D *AVFPhotoCatcherSteppable::TakeAPhoto_Implementation()
{
    auto Photo2D = Super::TakeAPhoto_Implementation();

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        StepsRecorder->SubmitStep(this, FVFStepInfo{TEXT("Take A Photo"), true});
    }
    return Photo2D;
}

bool AVFPhotoCatcherSteppable::StepBack_Implementation(FVFStepInfo &StepInfo)
{
    // Nothing To Do.
    return true;
}
