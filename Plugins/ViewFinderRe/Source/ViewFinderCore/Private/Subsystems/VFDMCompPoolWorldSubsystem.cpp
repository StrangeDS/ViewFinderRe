#include "VFDMCompPoolWorldSubsystem.h"

#include "UnrealEngine.h"

#include "VFCommon.h"
#include "VFDynamicMeshComponent.h"
#include "VFGeometryFunctions.h"

UVFDMCompPoolWorldSubsystem::UVFDMCompPoolWorldSubsystem()
{
}

void UVFDMCompPoolWorldSubsystem::Deinitialize()
{
    ClearComps();

    Super::Deinitialize();
}

UVFDynamicMeshComponent *UVFDMCompPoolWorldSubsystem::GetOrCreateComp(
    UObject *Outer,
    const TSubclassOf<UVFDynamicMeshComponent> &CompClass)
{
    UVFDynamicMeshComponent *CompRes = nullptr;
    if (!Outer || !CompClass)
        return CompRes;

    for (auto Comp : AvailableComps)
    {
        if (Comp->GetClass() == CompClass)
        {
            CompRes = Comp.Get();
            AvailableComps.RemoveSwap(Comp);
            CompRes->Rename(nullptr, Outer);
            return CompRes;
        }
    }

    CompRes = NewObject<UVFDynamicMeshComponent>(Outer, CompClass, NAME_None);
    AllComps.Add(CompRes);

    return CompRes;
}

void UVFDMCompPoolWorldSubsystem::ReturnComp(UVFDynamicMeshComponent *Comp)
{
    if (!AllComps.Contains(Comp))
        return;

    Comp->Rename(nullptr, this);
    AvailableComps.AddUnique(Comp);
}

void UVFDMCompPoolWorldSubsystem::ClearComps(bool bForceGarbage)
{
    AllComps.Reset(SizeOfPool);
    AvailableComps.Reset(SizeOfPool);
    if (bForceGarbage)
        GEngine->ForceGarbageCollection(true);
}
