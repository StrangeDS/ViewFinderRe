#include "VFPhoto2DSteppable.h"

#include "VFStepsRecorderWorldSubsystem.h"

void AVFPhoto2DSteppable::BeginPlay()
{
    Super::BeginPlay();

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        StepsRecorder->SubmitStep(
            this,
            FVFStepInfo{EnumToString<EVFPhoto2DSteppableOperation>(
                EVFPhoto2DSteppableOperation::None)});
    }
}

void AVFPhoto2DSteppable::FoldUp()
{
    bool FirstFold = State == EVFPhoto2DState::None;

    Super::FoldUp();

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        if (!FirstFold)
            StepsRecorder->SubmitStep(
                this,
                FVFStepInfo{EnumToString<EVFPhoto2DSteppableOperation>(
                    EVFPhoto2DSteppableOperation::Folded)});
    }
}

void AVFPhoto2DSteppable::PlaceDown()
{
    Super::PlaceDown();

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        StepsRecorder->SubmitStep(
            this,
            FVFStepInfo{EnumToString<EVFPhoto2DSteppableOperation>(
                EVFPhoto2DSteppableOperation::Placed)});
    }
}

bool AVFPhoto2DSteppable::ReattachToComponent(USceneComponent *Target)
{
    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        auto AttachParent = RootComponent->GetAttachParent();
        auto RelativeTransform = RootComponent->GetRelativeTransform();
        bool IsChanged = Super::ReattachToComponent(Target);
        if (IsChanged)
        {
            HierarchyRecorder.Add({AttachParent, RelativeTransform});

            StepsRecorder->SubmitStep(
                this,
                FVFStepInfo{EnumToString<EVFPhoto2DSteppableOperation>(
                    EVFPhoto2DSteppableOperation::ReattachTo)});
        }
        return IsChanged;
    }

    return Super::ReattachToComponent(Target);
}

bool AVFPhoto2DSteppable::StepBack_Implementation(FVFStepInfo &StepInfo)
{
    auto CompStep = StringToEnum<EVFPhoto2DSteppableOperation>(StepInfo.Info);
    switch (CompStep)
    {
    case EVFPhoto2DSteppableOperation::None:
    {
        Destroy();
        break;
    }
    case EVFPhoto2DSteppableOperation::Folded:
    {
        PlaceDown();
        break;
    }
    case EVFPhoto2DSteppableOperation::Placed:
    {
        SetActorEnableCollision(true);
        SetActorHiddenInGame(false);
        FoldUp();
        break;
    }
    case EVFPhoto2DSteppableOperation::ReattachTo:
    {
        auto HierarchyInfo = HierarchyRecorder.Pop(false);
        ReattachToComponent(HierarchyInfo.AttachParent);
        SetActorRelativeLocation(HierarchyInfo.RelativeTramsform.GetTranslation());
        SetActorRelativeRotation(HierarchyInfo.RelativeTramsform.GetRotation());
        break;
    }
    case EVFPhoto2DSteppableOperation::MAX:
    default:
        return false;
    }

    return true;
}