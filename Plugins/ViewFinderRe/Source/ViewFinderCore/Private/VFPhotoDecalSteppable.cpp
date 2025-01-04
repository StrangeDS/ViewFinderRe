#include "VFPhotoDecalSteppable.h"

void AVFPhotoDecalSteppable::BeginPlay()
{
    StepRecorder = GetWorld()->GetSubsystem<UVFStepsRecorderWorldSubsystem>();
    check(StepRecorder);

    Super::BeginPlay();
}

void AVFPhotoDecalSteppable::Replace()
{
    Super::Replace();

    if (!StepRecorder->bIsRewinding)
    {
        StepRecorder->SubmitStep(
            this,
            FVFStepInfo{
                EnumToString<AVFPhotoDecalOperation>(
                    AVFPhotoDecalOperation::Replace),
                true});
    }
}

void AVFPhotoDecalSteppable::Restore()
{
    Super::Restore();

    if (!StepRecorder->bIsRewinding)
    {
        StepRecorder->SubmitStep(
            this,
            FVFStepInfo{
                EnumToString<AVFPhotoDecalOperation>(
                    AVFPhotoDecalOperation::Restore),
                true});
    }
}

bool AVFPhotoDecalSteppable::StepBack_Implementation(FVFStepInfo &StepInfo)
{
    auto Step = StringToEnum<AVFPhotoDecalOperation>(StepInfo.Info);
    switch (Step)
    {
    case AVFPhotoDecalOperation::Replace:
    {
        Restore();
        break;
    }
    case AVFPhotoDecalOperation::Restore:
    {
        Replace();
        break;
    }
    default:
        return false;
    }

    return true;
}
