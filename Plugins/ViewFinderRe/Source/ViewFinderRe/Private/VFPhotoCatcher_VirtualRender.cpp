#include "VFPhotoCatcher_VirtualRender.h"

#include "Materials/MaterialParameterCollectionInstance.h"

AVFPhotoCatcher_VirtualRender::AVFPhotoCatcher_VirtualRender()
    : Super()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AVFPhotoCatcher_VirtualRender::BeginPlay()
{
    Super::BeginPlay();

    MPCI = GetWorld()->GetParameterCollectionInstance(MPC);
    MPCI->SetScalarParameterValue(TEXT("Radius"), RenderRadius);
}

void AVFPhotoCatcher_VirtualRender::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    MPCI->SetVectorParameterValue(TEXT("Center"), FLinearColor(GetActorLocation()));
}

void AVFPhotoCatcher_VirtualRender::DropDown_Implementation()
{
    if (bAddPPAfterDropDown)
    {
        auto PC = PlayerController;
        Super::DropDown_Implementation();
        PlayerController = PC;
        AddPostProcessToPlayerCamera();
        PlayerController = nullptr;
    }
    else
    {
        Super::DropDown_Implementation();
    }
    bAddPPAfterDropDown = !bAddPPAfterDropDown;
}