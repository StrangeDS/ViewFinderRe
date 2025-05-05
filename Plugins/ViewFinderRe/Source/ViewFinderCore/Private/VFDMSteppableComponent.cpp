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

    StepRecorder = GetWorld()->GetSubsystem<UVFStepsRecorderWorldSubsystem>();
    check(StepRecorder);

    Steps.Reserve(200);
    Steps.Add(FVFDMCompStep{
        UVFDMCompStepOperation::BeginPlay,
        nullptr,
        StepRecorder->Time});
    StepRecorder->RegisterTickable(this);

    LocalPool = NewObject<UDynamicMeshPool>(this);
}

void UVFDMSteppableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StepRecorder->UnregisterTickable(this);
    StepRecorder = nullptr;
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

    if (StepRecorder && !StepRecorder->bIsRewinding)
    {
        Steps.Add(FVFDMCompStep{
            UVFDMCompStepOperation::CopyMeshFromComponent,
            nullptr,
            StepRecorder->Time});

        if (bSimulatePhysicsRecorder)
        {
            StepRecorder->RecordTransform(this);

            Steps.Add(FVFDMCompStep{
                UVFDMCompStepOperation::RegisterToTransformRecorder,
                nullptr,
                StepRecorder->Time});
        }
    }
}

void UVFDMSteppableComponent::ReplaceMeshForComponent(UPrimitiveComponent *Source)
{
    Super::ReplaceMeshForComponent(Source);

    if (StepRecorder && !StepRecorder->bIsRewinding)
    {
        Steps.Add(FVFDMCompStep{
            UVFDMCompStepOperation::ReplaceMeshForComponent,
            nullptr,
            StepRecorder->Time});
    }
}

void UVFDMSteppableComponent::IntersectMeshWithDMComp(UDynamicMeshComponent *Tool)
{
    if (StepRecorder && !StepRecorder->bIsRewinding)
    {
        Steps.Add(FVFDMCompStep{
            UVFDMCompStepOperation::IntersectMeshWithDMComp,
            RequestACopiedMesh(),
            StepRecorder->Time});
    }

    Super::IntersectMeshWithDMComp(Tool);
}

void UVFDMSteppableComponent::SubtractMeshWithDMComp(UDynamicMeshComponent *Tool)
{
    if (StepRecorder && !StepRecorder->bIsRewinding)
    {
        Steps.Add(FVFDMCompStep{
            UVFDMCompStepOperation::SubtractMeshWithDMComp,
            RequestACopiedMesh(),
            StepRecorder->Time});
    }

    Super::SubtractMeshWithDMComp(Tool);
}

void UVFDMSteppableComponent::UnionMeshWithDMComp(UDynamicMeshComponent *Tool)
{
    if (StepRecorder && !StepRecorder->bIsRewinding)
    {
        Steps.Add(FVFDMCompStep{
            UVFDMCompStepOperation::UnionMeshWithDMComp,
            RequestACopiedMesh(),
            StepRecorder->Time});
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
            StepRecorder->UnrecordTransform(this);
            break;
        }
        case UVFDMCompStepOperation::ReplaceMeshForComponent:
        {
            SetEnabled(false);

            SourceComponent->SetHiddenInGame(false);
            SourceComponent->SetCollisionProfileName(GetCollisionProfileName());
            SourceComponent->SetCollisionEnabled(GetCollisionEnabled());
            SourceComponent->SetSimulatePhysics(bSimulatePhysicsRecorder);
            SourceComponent->BodyInstance.bSimulatePhysics = bEnableGravityRecorder;
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