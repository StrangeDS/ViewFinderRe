#include "VFTransformRecordVolume.h"

#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"

#include "VFHelperComponent.h"
#include "VFStepsRecordInterface.h"

AVFTransformRecordVolume::AVFTransformRecordVolume(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = false;
	Volume = CreateDefaultSubobject<UBoxComponent>("Volume");
    SetRootComponent(Volume);
    Volume->SetHiddenInGame(true);

    Helper = CreateDefaultSubobject<UVFHelperComponent>("Helper");
    Helper->bCanBeTakenInPhoto = false;
    Helper->bCanBePlacedByPhoto = false;
}

TArray<UPrimitiveComponent *> AVFTransformRecordVolume::GetComponents()
{
    TArray<UPrimitiveComponent *> OutComponents;
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
