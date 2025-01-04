#include "VFDMSteppableComponent.h"

#include "VFDynamicMeshPoolWorldSubsystem.h"

UVFDMSteppableComponent::UVFDMSteppableComponent(const FObjectInitializer &ObjectInitializer)
    : UVFDynamicMeshComponent(ObjectInitializer)
{
}

void UVFDMSteppableComponent::BeginPlay()
{
    Super::BeginPlay();

    StepRecorder = GetWorld()->GetSubsystem<UVFStepsRecorderWorldSubsystem>();
    check(StepRecorder);

    Steps.Reset();
    Steps.Add(FVFDMCompStep{
        UVFDMCompStepOperation::BeginPlay,
        nullptr,
        StepRecorder->Time});
    StepRecorder->RegisterTickable(this);
}

void UVFDMSteppableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (Steps.Num() > 1)
    {
        UE_LOG(LogTemp, Warning,
               TEXT("%s UVFDMSteppableComponent::EndPlay has UDynamicMesh not return %i"),
               *GetOwner()->GetName(),
               Steps.Num() - 1);
    }

    while (!Steps.IsEmpty())
    {
        auto &StepInfo = Steps.Last();
        if (StepInfo.Mesh)
            MeshPool->ReturnComputingMesh(StepInfo.Mesh);
        Steps.Pop(false);
    }

    Super::EndPlay(EndPlayReason);
}

UDynamicMesh *UVFDMSteppableComponent::RequestACopiedMesh()
{
    if (ensure(MeshPool))
    {
        auto CopyiedMesh = MeshPool->RequestComputingMesh();
        CopyiedMesh->SetMesh(MeshObject->GetMeshRef());
        return CopyiedMesh;
    }
    return nullptr;
}

void UVFDMSteppableComponent::CopyMeshFromComponent(UPrimitiveComponent *Source)
{
    Super::CopyMeshFromComponent(Source);


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

void UVFDMSteppableComponent::ReplaceMeshForComponent(UPrimitiveComponent *Source)
{
    Super::ReplaceMeshForComponent(Source);


    Steps.Add(FVFDMCompStep{
        UVFDMCompStepOperation::ReplaceMeshForComponent,
        nullptr,
        StepRecorder->Time});
}

void UVFDMSteppableComponent::IntersectMeshWithDMComp(UDynamicMeshComponent *Tool)
{
    Steps.Add(FVFDMCompStep{
        UVFDMCompStepOperation::IntersectMeshWithDMComp,
        RequestACopiedMesh(),
        StepRecorder->Time});

    Super::IntersectMeshWithDMComp(Tool);
}

void UVFDMSteppableComponent::SubtractMeshWithDMComp(UDynamicMeshComponent *Tool)
{
    Steps.Add(FVFDMCompStep{
        UVFDMCompStepOperation::SubtractMeshWithDMComp,
        RequestACopiedMesh(),
        StepRecorder->Time});

    Super::SubtractMeshWithDMComp(Tool);
}

void UVFDMSteppableComponent::UnionMeshWithDMComp(UDynamicMeshComponent *Tool)
{
    Steps.Add(FVFDMCompStep{
        UVFDMCompStepOperation::UnionMeshWithDMComp,
        RequestACopiedMesh(),
        StepRecorder->Time});

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
            // Source不保存的做法
            // auto SourceVFDMComp = GetSourceVFDMComp();
            // if (SourceVFDMComp)
            // {
            //     SetComponentToWorld(SourceVFDMComp->GetComponentToWorld());
            //     SourceVFDMComp->UnionMeshWithDMComp(this);
            // }
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
            MeshPool->ReturnComputingMesh(StepInfo.Mesh);
            break;
        }
        default:
            break;
        }
        Steps.Pop(false);
    }
}