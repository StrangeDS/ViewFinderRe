// Copyright StrangeDS. All Rights Reserved.

#include "VFPhotoContainerSteppable.h"

#include "VFPhoto2D.h"
#include "VFStepsRecorderWorldSubsystem.h"

void AVFPhotoContainerSteppable::BeginPlay()
{
    Super::BeginPlay();

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        StepsRecorder->RegisterTickable(this);
        StepsRecorder->OnEndRewinding.AddUniqueDynamic(this,
                                                       &AVFPhotoContainerSteppable::HandleEndRewinding);
    }

    Steps.Reserve(UVFStepsRecorderWorldSubsystem::GetSizeRecommended());
}

void AVFPhotoContainerSteppable::AddAPhoto(AVFPhoto2D *Photo)
{
    Super::AddAPhoto(Photo);

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        Steps.Add(FVFPhotoContainerStepInfo{
            EVFPhotoContainerSteppableOperation::Add,
            Photo,
            StepsRecorder->GetTime()});
    }
}

void AVFPhotoContainerSteppable::PlaceCurrentPhoto()
{
    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        if (IsValid(CurrentPhoto2D))
        {
            StepsRecorder->SubmitStep(this,
                                      FVFStepInfo{TEXT("PlaceCurrentPhoto"), true});
            auto Photo2D = CurrentPhoto2D;
            Super::PlaceCurrentPhoto();

            Steps.Add(FVFPhotoContainerStepInfo{
                EVFPhotoContainerSteppableOperation::Place,
                Photo2D,
                StepsRecorder->GetTime()});
            return;
        }
    }

    Super::PlaceCurrentPhoto();
}

void AVFPhotoContainerSteppable::PrepareCurrentPhoto()
{
    Super::PrepareCurrentPhoto();

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        Steps.Add(FVFPhotoContainerStepInfo{
            EVFPhotoContainerSteppableOperation::Prepare,
            CurrentPhoto2D,
            StepsRecorder->GetTime()});
    }
}

void AVFPhotoContainerSteppable::GiveUpPreparing()
{
    Super::GiveUpPreparing();

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        Steps.Add(FVFPhotoContainerStepInfo{
            EVFPhotoContainerSteppableOperation::GiveUpPreparing,
            CurrentPhoto2D,
            StepsRecorder->GetTime()});
    }
}

void AVFPhotoContainerSteppable::ChangeCurrentPhoto(const bool Next)
{
    Super::ChangeCurrentPhoto(Next);

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        Steps.Add(FVFPhotoContainerStepInfo{
            Next ? EVFPhotoContainerSteppableOperation::ChangeNext
                 : EVFPhotoContainerSteppableOperation::ChangeLast,
            CurrentPhoto2D,
            StepsRecorder->GetTime()});
    }
}

void AVFPhotoContainerSteppable::SetEnabled(const bool &Enabled)
{
    Super::SetEnabled(Enabled);

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        Steps.Add(FVFPhotoContainerStepInfo{
            Enabled ? EVFPhotoContainerSteppableOperation::Enabled
                    : EVFPhotoContainerSteppableOperation::Disabled,
            CurrentPhoto2D,
            StepsRecorder->GetTime()});
    }
}

bool AVFPhotoContainerSteppable::StepBack_Implementation(FVFStepInfo &StepInfo)
{
    auto Step = StringToEnum<EVFPhotoContainerSteppableOperation>(StepInfo.Info);
    switch (Step)
    {
    case EVFPhotoContainerSteppableOperation::Prepare:
    {
        GiveUpPreparing();
        break;
    }
    case EVFPhotoContainerSteppableOperation::GiveUpPreparing:
    {
        PrepareCurrentPhoto();
        break;
    }
    case EVFPhotoContainerSteppableOperation::ChangeNext:
    {
        ChangeCurrentPhoto(false);
        break;
    }
    case EVFPhotoContainerSteppableOperation::ChangeLast:
    {
        ChangeCurrentPhoto(true);
        break;
    }
    case EVFPhotoContainerSteppableOperation::Enabled:
    {
        SetEnabled(false);
        break;
    }
    case EVFPhotoContainerSteppableOperation::Disabled:
    {
        SetEnabled(true);
        break;
    }
    default:
    {
        if (StepInfo.Info == TEXT("PlaceCurrentPhoto"))
        {
            // Nothing to do.
            return true;
        }
        return false;
    }
    }

    return true;
}

void AVFPhotoContainerSteppable::HandleEndRewinding(float Time)
{
    if (bFocusOn)
        GiveUpPreparing();
}

void AVFPhotoContainerSteppable::TickBackward_Implementation(float Time)
{
    while (Steps.Num())
    {
        FVFPhotoContainerStepInfo &StepInfo = Steps.Last();
        if (StepInfo.Time < Time)
            break;

        switch (StepInfo.Operation)
        {
        case EVFPhotoContainerSteppableOperation::Add:
        {
            auto &Photo = StepInfo.Photo;
            if (ensure(Photo2Ds.Last() == StepInfo.Photo))
            {
                Photo->SetActorHiddenInGame(false);
                Photo2Ds.PopLast();
                Photo->SetActorEnableCollision(true);
                UpdateCurrentPhoto();
            }
            break;
        }
        case EVFPhotoContainerSteppableOperation::Place:
        {
            if (IsValid(CurrentPhoto2D))
                CurrentPhoto2D->SetActorHiddenInGame(true);
            auto &Photo = StepInfo.Photo;
            AddAPhoto(Photo);
            SetEnabled(true);
            break;
        }
        case EVFPhotoContainerSteppableOperation::Prepare:
        {
            GiveUpPreparing();
            break;
        }
        case EVFPhotoContainerSteppableOperation::GiveUpPreparing:
        {
            PrepareCurrentPhoto();
            break;
        }
        case EVFPhotoContainerSteppableOperation::ChangeNext:
        {
            ChangeCurrentPhoto(false);
            break;
        }
        case EVFPhotoContainerSteppableOperation::ChangeLast:
        {
            ChangeCurrentPhoto(true);
            break;
        }
        case EVFPhotoContainerSteppableOperation::Enabled:
        {
            SetEnabled(false);
            break;
        }
        case EVFPhotoContainerSteppableOperation::Disabled:
        {
            SetEnabled(true);
            break;
        }
        case EVFPhotoContainerSteppableOperation::MAX:
        default:
            return;
        }

        Steps.Pop(false);
    }
}