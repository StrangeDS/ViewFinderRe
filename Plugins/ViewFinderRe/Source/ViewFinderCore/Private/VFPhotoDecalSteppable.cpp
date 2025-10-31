#include "VFPhotoDecalSteppable.h"

#include "VFStepsRecorderWorldSubsystem.h"

void AVFPhotoDecalSteppable::BeginPlay()
{
    Super::BeginPlay();

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        float Time = bPersistent ? StepsRecorder->GetTimeOfMin() : -1.0f;
        StepsRecorder->SubmitStep(
            this,
            FVFStepInfo{
                EnumToString<AVFPhotoDecalOperation>(
                    AVFPhotoDecalOperation::None),
                true,
                Time});
    }
}

void AVFPhotoDecalSteppable::ReplaceWithDecal_Implementation(bool bUpdateRT)
{
    Super::ReplaceWithDecal_Implementation(bUpdateRT);

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        float Time = StepsRecorder->GetTime();
        if (bKeepFirstReplacement && bIsFirstReplacement)
        {
            Time = StepsRecorder->GetTimeOfMin();
            bIsFirstReplacement = false;
        }

        StepsRecorder->SubmitStep(
            this,
            FVFStepInfo{
                EnumToString<AVFPhotoDecalOperation>(
                    AVFPhotoDecalOperation::Replace),
                true,
                Time});
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
    case AVFPhotoDecalOperation::None:
    {
        Destroy();
        break;
    }
    case AVFPhotoDecalOperation::Replace:
    {
        RestoreWithActors();
        break;
    }
    case AVFPhotoDecalOperation::Restore:
    {
        ReplaceWithDecal(false);
        break;
    }
    default:
        return false;
    }

    return true;
}
