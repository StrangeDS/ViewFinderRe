#include "VFPhotoContainerSteppable.h"

#include "VFPhoto2D.h"

void AVFPhotoContainerSteppable::BeginPlay()
{
    Super::BeginPlay();

    StepRecorder = GetWorld()->GetSubsystem<UVFStepsRecorderWorldSubsystem>();
    check(StepRecorder);
    StepRecorder->RegisterTickable(this);
}

void AVFPhotoContainerSteppable::AddAPhoto(AVFPhoto2D *Photo)
{
    Super::AddAPhoto(Photo);

    if (!StepRecorder->bIsRewinding)
    {
        Steps.Add(FVFPhotoContainerStepInfo{
            AVFPhotoContainerSteppableOperation::Add,
            Photo,
            StepRecorder->GetTime()});
    }
}

void AVFPhotoContainerSteppable::PlaceCurrentPhoto()
{
    if (!StepRecorder->bIsRewinding)
    {
        StepRecorder->SubmitStep(
            this,
            FVFStepInfo{
                TEXT("Place a Photo"),
                true});
        Steps.Add(FVFPhotoContainerStepInfo{
            AVFPhotoContainerSteppableOperation::Place,
            CurrentPhoto2D,
            StepRecorder->GetTime()});
    }

    Super::PlaceCurrentPhoto();
}

void AVFPhotoContainerSteppable::PrepareCurrentPhoto(float Time)
{
    Super::PrepareCurrentPhoto(Time);

    if (!StepRecorder->bIsRewinding)
    {
        Steps.Add(FVFPhotoContainerStepInfo{
            AVFPhotoContainerSteppableOperation::Prepare,
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
            AVFPhotoContainerSteppableOperation::GiveUpPreparing,
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
            Next ? AVFPhotoContainerSteppableOperation::ChangeNext
                 : AVFPhotoContainerSteppableOperation::ChangeLast,
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
            Enabled ? AVFPhotoContainerSteppableOperation::Enabled
                    : AVFPhotoContainerSteppableOperation::Disabled,
            CurrentPhoto2D,
            StepRecorder->GetTime()});
    }
}

bool AVFPhotoContainerSteppable::StepBack_Implementation(FVFStepInfo &StepInfo)
{
    auto Step = StringToEnum<AVFPhotoContainerSteppableOperation>(StepInfo.Info);
    switch (Step)
    {
    case AVFPhotoContainerSteppableOperation::Prepare:
    {
        GiveUpPreparing();
        break;
    }
    case AVFPhotoContainerSteppableOperation::GiveUpPreparing:
    {
        PrepareCurrentPhoto();
        break;
    }
    case AVFPhotoContainerSteppableOperation::ChangeNext:
    {
        ChangeCurrentPhoto(false);
        break;
    }
    case AVFPhotoContainerSteppableOperation::ChangeLast:
    {
        ChangeCurrentPhoto(true);
        break;
    }
    case AVFPhotoContainerSteppableOperation::Enabled:
    {
        SetEnabled(false);
        break;
    }
    case AVFPhotoContainerSteppableOperation::Disabled:
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
        case AVFPhotoContainerSteppableOperation::Add:
        {
            auto &Photo = StepInfo.Photo;
            if (ensure(Photo2Ds.Last() == StepInfo.Photo))
            {
                Photo->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
                Photo2Ds.PopLast();
                UpdateCurrentPhoto();
                Photo->SetActorHiddenInGame(false);
            }
            break;
        }
        case AVFPhotoContainerSteppableOperation::Place:
        {
            auto &Photo = StepInfo.Photo;
            AddAPhoto(Photo);
            SetEnabled(true);
            GiveUpPreparing();
            break;
        }
        case AVFPhotoContainerSteppableOperation::Prepare:
        {
            GiveUpPreparing();
            break;
        }
        case AVFPhotoContainerSteppableOperation::GiveUpPreparing:
        {
            PrepareCurrentPhoto();
            break;
        }
        case AVFPhotoContainerSteppableOperation::ChangeNext:
        {
            ChangeCurrentPhoto(false);
            break;
        }
        case AVFPhotoContainerSteppableOperation::ChangeLast:
        {
            ChangeCurrentPhoto(true);
            break;
        }
        case AVFPhotoContainerSteppableOperation::Enabled:
        {
            SetEnabled(false);
            break;
        }
        case AVFPhotoContainerSteppableOperation::Disabled:
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