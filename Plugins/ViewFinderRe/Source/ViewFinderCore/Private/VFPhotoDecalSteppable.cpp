#include "VFPhotoDecalSteppable.h"

#include "TimerManager.h"

#include "VFStepsRecorderWorldSubsystem.h"

void AVFPhotoDecalSteppable::BeginPlay()
{
    Super::BeginPlay();
}

void AVFPhotoDecalSteppable::ReplaceWithDecal_Implementation()
{
    Super::ReplaceWithDecal_Implementation();

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        StepsRecorder->SubmitStep(
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

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        StepsRecorder->SubmitStep(
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
