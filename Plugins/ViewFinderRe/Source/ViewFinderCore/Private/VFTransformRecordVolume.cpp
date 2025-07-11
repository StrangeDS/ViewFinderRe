#include "VFTransformRecordVolume.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"

#include "VFHelperComponent.h"
#include "VFStepsRecordInterface.h"
#include "VFStepsRecorderWorldSubsystem.h"

AVFTransformRecordVolume::AVFTransformRecordVolume(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = false;
    Volume = CreateDefaultSubobject<UBoxComponent>("Volume");
    SetRootComponent(Volume);
    Volume->SetHiddenInGame(true);

    CompClass = UStaticMeshComponent::StaticClass();

    Helper = CreateDefaultSubobject<UVFHelperComponent>("Helper");
	Helper->ShowInPhotoRule = FVFShowInPhotoRule::Neither;
    Helper->bCanBeTakenInPhoto = false;
    Helper->bCanBePlacedByPhoto = false;
}

void AVFTransformRecordVolume::BeginPlay()
{
    Super::BeginPlay();

    Helper->OnCopyEndPlacingPhoto.AddUniqueDynamic(
        this,
        &AVFTransformRecordVolume::HandleCopyEndPlacingPhoto);
}

TArray<UPrimitiveComponent *> AVFTransformRecordVolume::GetComponents()
{
    TArray<UPrimitiveComponent *> OutComponents;
    if (!bEnabled)
        return OutComponents;

    bool Result = UKismetSystemLibrary::ComponentOverlapComponents(
        Volume,
        Volume->GetComponentTransform(),
        ObjectTypes,
        CompClass,
        {},
        OutComponents);

    for (auto It = OutComponents.CreateIterator(); It; It++)
    {
        auto &Comp = (*It);
        if (Comp->Implements<UVFStepsRecordInterface>())
            It.RemoveCurrent();
        if (Comp->Mobility != EComponentMobility::Movable)
            It.RemoveCurrent();
    }

    return OutComponents;
}

UVFHelperComponent *AVFTransformRecordVolume::GetHelper_Implementation()
{
    return Helper;
}

void AVFTransformRecordVolume::HandleCopyEndPlacingPhoto(UObject *Sender)
{
    if (auto StepsRecorder = UVFStepsRecorderWorldSubsystem::GetStepsRecorder(this))
    {
        TArray<UPrimitiveComponent *> OutComponents;
        bool Result = UKismetSystemLibrary::ComponentOverlapComponents(
            Volume,
            Volume->GetComponentTransform(),
            ObjectTypes,
            CompClass,
            {},
            OutComponents);

        for (auto Comp : OutComponents)
        {
            StepsRecorder->RecordTransform(Comp);
        }
    }
}