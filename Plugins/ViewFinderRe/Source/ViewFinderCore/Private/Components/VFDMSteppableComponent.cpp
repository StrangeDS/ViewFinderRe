#include "VFDMSteppableComponent.h"

#include "Engine/World.h"

#include "VFCommon.h"
#include "VFDMCompPoolWorldSubsystem.h"
#include "VFStepsRecorderWorldSubsystem.h"

UVFDMSteppableComponent::UVFDMSteppableComponent(const FObjectInitializer &ObjectInitializer)
    : UVFDynamicMeshComponent(ObjectInitializer)
{
}

void UVFDMSteppableComponent::BeginPlay()
{
    Super::BeginPlay();

    Steps.Reserve(200);
    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        Steps.Add(FVFDMCompStep{
            UVFDMCompStepOperation::BeginPlay,
            nullptr,
            StepsRecorder->Time});
        StepsRecorder->RegisterTickable(this);
    }

    LocalPool = NewObject<UDynamicMeshPool>(this);
}

void UVFDMSteppableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        StepsRecorder->UnregisterTickable(this);
    }
    Steps.Reset();
    LocalPool->ReturnAllMeshes();

    Super::EndPlay(EndPlayReason);
}

void UVFDMSteppableComponent::DestroyComponent(bool bPromoteChildren)
{
    auto World = GetWorld();

    Super::DestroyComponent(bPromoteChildren);

    if (auto CompsPool = World->GetSubsystem<UVFDMCompPoolWorldSubsystem>())
    {
        CompsPool->ReturnComp(this);
    }
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
        case UVFDMCompStepOperation::BeginPlay:
        {
            if (GetSourceVFDMComp()) // 复制体上的
                GetOwner()->Destroy();
            else
                DestroyComponent(); // 原网格上的(ReplaceMeshForComponent)创建的
            return;
        }
        case UVFDMCompStepOperation::CopyMeshFromComponent:
        {
            break;
        }
        case UVFDMCompStepOperation::RegisterToTransformRecorder:
        {
            auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this);
            if (ensure(StepsRecorder))
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