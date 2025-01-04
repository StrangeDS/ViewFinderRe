#include "VFPhoto2DSteppable.h"

void AVFPhoto2DSteppable::BeginPlay()
{
    Super::BeginPlay();

    StepRecorder = GetWorld()->GetSubsystem<UVFStepsRecorderWorldSubsystem>();
    check(StepRecorder);
    StepRecorder->SubmitStep(
        this,
        FVFStepInfo{EnumToString<EVFPhoto2DState>(
            EVFPhoto2DState::None)});
}

void AVFPhoto2DSteppable::FoldUp()
{
    Super::FoldUp();

    if (!StepRecorder->bIsRewinding)
    {
        StepRecorder->SubmitStep(
            this,
            FVFStepInfo{EnumToString<EVFPhoto2DState>(
                EVFPhoto2DState::Folded)});
    }
}

void AVFPhoto2DSteppable::PlaceDown()
{
    Super::PlaceDown();

    if (!StepRecorder->bIsRewinding)
    {
        StepRecorder->SubmitStep(
            this,
            FVFStepInfo{EnumToString<EVFPhoto2DState>(
                EVFPhoto2DState::Placed)});
    }
}

bool AVFPhoto2DSteppable::StepBack_Implementation(FVFStepInfo &StepInfo)
{
    auto CompStep = StringToEnum<EVFPhoto2DState>(StepInfo.Info);
    switch (CompStep)
    {
    case EVFPhoto2DState::None:
    {
        Destroy();
        break;
    }
    case EVFPhoto2DState::Folded:
    {
        PlaceDown();
        break;
    }
    case EVFPhoto2DState::Placed:
    {
        FoldUp();
        break;
    }
    default:
        return false;
    }

    return true;
}