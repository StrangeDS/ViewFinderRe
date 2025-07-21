#include "VFDMSteppableComponent.h"

#include "Engine/World.h"
#include "TimerManager.h"

#include "VFCommon.h"
#include "VFDMCompPoolWorldSubsystem.h"
#include "VFStepsRecorderWorldSubsystem.h"
#include "VFFunctions.h"

UVFDMSteppableComponent::UVFDMSteppableComponent(const FObjectInitializer &ObjectInitializer)
    : UVFDynamicMeshComponent(ObjectInitializer)
{
}

void UVFDMSteppableComponent::BeginPlay()
{
    Super::BeginPlay();

    LocalPool = NewObject<UDynamicMeshPool>(this);
}

void UVFDMSteppableComponent::Init(UPrimitiveComponent *Source)
{
    Super::Init(Source);

    Steps.Reserve(200);
    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        Steps.Add(FVFDMCompStep{
            UVFDMCompStepOperation::Init,
            nullptr,
            StepsRecorder->Time});
        StepsRecorder->RegisterTickable(this);

        if (bSimulatePhysicsRecorder)
        {
            StepsRecorder->RecordTransform(this);

            Steps.Add(FVFDMCompStep{
                UVFDMCompStepOperation::RegisterToTransformRecorder,
                nullptr,
                StepsRecorder->Time});
        }
    }
}

void UVFDMSteppableComponent::Clear()
{
    // 对于已经在池中的组件, GetWorld()是nullptr, 故需要这层保护.
    if (auto World = GetWorld())
    {
        if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
        {
            if (StepsRecorder->IsTickableRegistered(this))
                StepsRecorder->UnregisterTickable(this);
        }
    }
    Steps.Reset();
    LocalPool->ReturnAllMeshes();

    Super::Clear();
}

UDynamicMesh *UVFDMSteppableComponent::RequestACopiedMesh()
{
    auto CopyiedMesh = LocalPool->RequestMesh();
    CopyiedMesh->SetMesh(MeshObject->GetMeshRef());
    return CopyiedMesh;
}

void UVFDMSteppableComponent::CopyMeshFromComponent(UPrimitiveComponent *Source)
{
    Super::CopyMeshFromComponent(Source);

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        Steps.Add(FVFDMCompStep{
            UVFDMCompStepOperation::CopyMeshFromComponent,
            nullptr,
            StepsRecorder->Time});
    }
}

void UVFDMSteppableComponent::ReplaceMeshForComponent(UPrimitiveComponent *Source)
{
    Super::ReplaceMeshForComponent(Source);

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        Steps.Add(FVFDMCompStep{
            UVFDMCompStepOperation::ReplaceMeshForComponent,
            nullptr,
            StepsRecorder->Time});
    }
}

void UVFDMSteppableComponent::IntersectMeshWithDMComp(UDynamicMeshComponent *Tool)
{
    Super::IntersectMeshWithDMComp(Tool);

    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        Steps.Add(FVFDMCompStep{
            UVFDMCompStepOperation::IntersectMeshWithDMComp,
            RequestACopiedMesh(),
            StepsRecorder->Time});
    }
}

void UVFDMSteppableComponent::SubtractMeshWithDMComp(UDynamicMeshComponent *Tool)
{
    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        Steps.Add(FVFDMCompStep{
            UVFDMCompStepOperation::SubtractMeshWithDMComp,
            RequestACopiedMesh(),
            StepsRecorder->Time});
    }

    Super::SubtractMeshWithDMComp(Tool);
}

void UVFDMSteppableComponent::UnionMeshWithDMComp(UDynamicMeshComponent *Tool)
{
    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        Steps.Add(FVFDMCompStep{
            UVFDMCompStepOperation::UnionMeshWithDMComp,
            RequestACopiedMesh(),
            StepsRecorder->Time});
    }

    Super::UnionMeshWithDMComp(Tool);
}

void UVFDMSteppableComponent::TickBackward_Implementation(float Time)
{
    while (!Steps.IsEmpty())
    {
        auto &StepInfo = Steps.Last();
        if (StepInfo.Time < Time)
            break;

        switch (StepInfo.Operation)
        {
        case UVFDMCompStepOperation::Init:
        {
            auto Actor = GetOwner();
            bool NeedToDestroyActor = false;
            auto PoolSubsystem = GetWorld()->GetSubsystem<UVFDMCompPoolWorldSubsystem>();
            if (GetSourceVFDMComp())
            {
                TInlineComponentArray<UVFDMSteppableComponent *> VFDMComps(Actor);
                Actor->GetComponents<UVFDMSteppableComponent>(VFDMComps);
                NeedToDestroyActor = VFDMComps.Num() == 1;
            }

            DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
            UnregisterComponent();
            Actor->RemoveInstanceComponent(this);
            Clear();

            if (!UVFFunctions::IsEditorCreated(this) && PoolSubsystem)
            {
                if (!PoolSubsystem->ReturnComp(this))
                {
                    VF_LOG(Error, TEXT("%s: fails to ReturnComp."), __FUNCTIONW__);
                }
            }
            else
            {
                DestroyComponent(false);
            }

            if (NeedToDestroyActor)
            {
                Actor->Destroy();
            }
            return;
        }
        case UVFDMCompStepOperation::CopyMeshFromComponent:
        {
            break;
        }
        case UVFDMCompStepOperation::RegisterToTransformRecorder:
        {
            auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(
                this,
                EVFStepsRecorderSubsystemCheckMode::IgnoreRewinding);
            if (ensure(IsValid(StepsRecorder)))
            {
                StepsRecorder->UnrecordTransform(this);
            }
            break;
        }
        case UVFDMCompStepOperation::ReplaceMeshForComponent:
        {
            SetEnabled(false);
            RestoreSourceComponent();
            break;
        }
        case UVFDMCompStepOperation::IntersectMeshWithDMComp:
        case UVFDMCompStepOperation::SubtractMeshWithDMComp:
        case UVFDMCompStepOperation::UnionMeshWithDMComp:
        {
            MeshObject->SetMesh(StepInfo.Mesh->GetMeshRef());
            break;
        }
        default:
            break;
        }
        Steps.Pop(false);
    }
}