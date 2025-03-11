#include "VFPhotoDecalSteppable.h"

void AVFPhotoDecalSteppable::BeginPlay()
{
    StepRecorder = GetWorld()->GetSubsystem<UVFStepsRecorderWorldSubsystem>();
    check(StepRecorder);

    Super::BeginPlay();
}

void AVFPhotoDecalSteppable::ReplaceWithDecal_Implementation()
{
    Super::ReplaceWithDecal_Implementation();

    if (StepRecorder && !StepRecorder->bIsRewinding)
    {
        StepRecorder->SubmitStep(
            this,
            FVFStepInfo{
                EnumToString<AVFPhotoDecalOperation>(
                    AVFPhotoDecalOperation::Replace),
                true});
    }
}

void AVFPhotoDecalSteppable::RestoreWithActors_Implementation()
{
    Super::RestoreWithActors_Implementation();

    if (StepRecorder && !StepRecorder->bIsRewinding)
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
        RestoreWithActors();
        break;
    }
    case AVFPhotoDecalOperation::Restore:
    {
        // 拍照需要在下一帧进行(还原的网格还没更新)
        GetWorldTimerManager().SetTimerForNextTick(
            this,
            &AVFPhotoDecalSteppable::ReplaceWithDecal);
        break;
    }
    default:
        return false;
    }

    return true;
}
