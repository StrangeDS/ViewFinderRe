#include "VFPhoto3DSteppable.h"

#include "VFStepsRecorderWorldSubsystem.h"

void AVFPhoto3DSteppable::BeginPlay()
{
    Super::BeginPlay();

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        StepsRecorder->SubmitStep(
            this,
            FVFStepInfo{EnumToString<EVFPhoto3DState>(
                EVFPhoto3DState::None)});
    }
}

void AVFPhoto3DSteppable::FoldUp()
{
    bool FirstFold = State == EVFPhoto3DState::None;

    Super::FoldUp();

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        StepsRecorder->SubmitStep(
            this,
            FVFStepInfo{EnumToString<EVFPhoto3DState>(
                FirstFold ? EVFPhoto3DState::FirstFold
                          : EVFPhoto3DState::Folded)});
    }
}

void AVFPhoto3DSteppable::PlaceDown()
{
    Super::PlaceDown();

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        StepsRecorder->SubmitStep(
            this,
            FVFStepInfo{EnumToString<EVFPhoto3DState>(
                EVFPhoto3DState::Placed)});
    }
}

bool AVFPhoto3DSteppable::StepBack_Implementation(FVFStepInfo &StepInfo)
{
    auto CompStep = StringToEnum<EVFPhoto3DState>(StepInfo.Info);
    switch (CompStep)
    {
    case EVFPhoto3DState::None:
    {
        Destroy();
        break;
    }
    case EVFPhoto3DState::FirstFold:
    {
        // Nothing to do.
        break;
    }
    case EVFPhoto3DState::Folded:
    {
        PlaceDown();
        break;
    }
    case EVFPhoto3DState::Placed:
    {
        FoldUp();
        break;
    }
    default:
        return false;
    }

    return true;
}