#include "VFDynamicMeshPoolWorldSubsystem.h"

#include "VFCommon.h"
#include "VFDynamicMeshComponent.h"
#include "VFGeometryFunctions.h"

UVFDynamicMeshPoolWorldSubsystem::UVFDynamicMeshPoolWorldSubsystem()
{
}

void UVFDynamicMeshPoolWorldSubsystem::Deinitialize()
{
    ClearComps();

    Super::Deinitialize();
}

UVFDynamicMeshComponent *UVFDynamicMeshPoolWorldSubsystem::GetOrCreateComp(
    UObject *Outer,
    const TSubclassOf<UVFDynamicMeshComponent> &CompClass)
{
    UVFDynamicMeshComponent *CompRes = nullptr;
    if (!Outer || !CompClass)
        return CompRes;

    if (bUsingPool)
    {
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
    }

    CompRes = NewObject<UVFDynamicMeshComponent>(Outer, CompClass, NAME_None);
    AllComps.Add(CompRes);

    return CompRes;
}

void UVFDynamicMeshPoolWorldSubsystem::ReturnComp(UVFDynamicMeshComponent *Comp)
{
    if (!bUsingPool || !AllComps.Contains(Comp))
        return;

    Comp->Rename(nullptr, this);
    AvailableComps.AddUnique(Comp);
}

void UVFDynamicMeshPoolWorldSubsystem::ClearComps(bool bForceGarbage)
{
    AllComps.Reset(SizeOfPool);
    AvailableComps.Reset(SizeOfPool);
    if (bForceGarbage)
        GEngine->ForceGarbageCollection(true);
}
