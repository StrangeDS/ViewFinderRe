#include "VFPhotoContainerSteppable.h"

#include "VFPhoto2D.h"
#include "VFStepsRecorderWorldSubsystem.h"

void AVFPhotoContainerSteppable::BeginPlay()
{
    Super::BeginPlay();

    StepRecorder = GetWorld()->GetSubsystem<UVFStepsRecorderWorldSubsystem>();
    check(StepRecorder);
    StepRecorder->RegisterTickable(this);

    Steps.Reserve(UVFStepsRecorderWorldSubsystem::SizeRecommended);
}

void AVFPhotoContainerSteppable::AddAPhoto(AVFPhoto2D *Photo)
{
    Super::AddAPhoto(Photo);

    if (!StepRecorder->bIsRewinding)
    {
        Steps.Add(FVFPhotoContainerStepInfo{
            EVFPhotoContainerSteppableOperation::Add,
            Photo,
            StepRecorder->GetTime()});
    }
}

void AVFPhotoContainerSteppable::PlaceCurrentPhoto()
{
    if (!StepRecorder->bIsRewinding && CurrentPhoto2D)
    {
        auto Photo2D = CurrentPhoto2D;
        Super::PlaceCurrentPhoto();

        Steps.Add(FVFPhotoContainerStepInfo{
            EVFPhotoContainerSteppableOperation::Place,
            Photo2D,
            StepRecorder->GetTime()});
        return;
    }

    Super::PlaceCurrentPhoto();
}

void AVFPhotoContainerSteppable::PrepareCurrentPhoto()
{
    Super::PrepareCurrentPhoto();

    if (!StepRecorder->bIsRewinding)
    {
        Steps.Add(FVFPhotoContainerStepInfo{
            EVFPhotoContainerSteppableOperation::Prepare,
            CurrentPhoto2D,
            StepRecorder->GetTime()});
    }
}

void AVFPhotoContainerSteppable::GiveUpPreparing()
{
    Super::GiveUpPreparing();

    if (!StepRecorder->bIsRewinding)
    {
        Steps.Add(FVFPhotoContainerStepInfo{
            EVFPhotoContainerSteppableOperation::GiveUpPreparing,
            CurrentPhoto2D,
            StepRecorder->GetTime()});
    }
}

void AVFPhotoContainerSteppable::ChangeCurrentPhoto(const bool Next)
{
    Super::ChangeCurrentPhoto(Next);

    if (!StepRecorder->bIsRewinding)
    {
        Steps.Add(FVFPhotoContainerStepInfo{
            Next ? EVFPhotoContainerSteppableOperation::ChangeNext
                 : EVFPhotoContainerSteppableOperation::ChangeLast,
            CurrentPhoto2D,
            StepRecorder->GetTime()});
    }
}

void AVFPhotoContainerSteppable::SetEnabled(const bool &Enabled)
{
    Super::SetEnabled(Enabled);

    if (!StepRecorder->bIsRewinding)
    {
        Steps.Add(FVFPhotoContainerStepInfo{
            Enabled ? EVFPhotoContainerSteppableOperation::Enabled
                    : EVFPhotoContainerSteppableOperation::Disabled,
            CurrentPhoto2D,
            StepRecorder->GetTime()});
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
        return false;
    }

    return true;
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
            if (CurrentPhoto2D)
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
        default:
            return;
        }

        Steps.Pop(false);
    }
}