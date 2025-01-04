#include "VFPhoto3DSteppable.h"

void AVFPhoto3DSteppable::BeginPlay()
{
    StepRecorder = GetWorld()->GetSubsystem<UVFStepsRecorderWorldSubsystem>();
    check(StepRecorder);
    StepRecorder->SubmitStep(
        this,
        FVFStepInfo{EnumToString<EVFPhoto3DState>(
            EVFPhoto3DState::None)});

    Super::BeginPlay();
}

void AVFPhoto3DSteppable::FoldUp()
{
    bool FirstFold = State == EVFPhoto3DState::None;

    Super::FoldUp();

    if (!StepRecorder->bIsRewinding)
    {
        StepRecorder->SubmitStep(
            this,
            FVFStepInfo{EnumToString<EVFPhoto3DState>(
                FirstFold ? EVFPhoto3DState::FirstFold
                          : EVFPhoto3DState::Folded)});
    }
}

void AVFPhoto3DSteppable::PlaceDown()
{
    Super::PlaceDown();

    if (!StepRecorder->bIsRewinding)
    {
        StepRecorder->SubmitStep(
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