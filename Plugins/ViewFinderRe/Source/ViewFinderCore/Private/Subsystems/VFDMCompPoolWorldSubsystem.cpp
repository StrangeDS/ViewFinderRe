#include "VFDMCompPoolWorldSubsystem.h"

#include "UnrealEngine.h"

#include "VFDynamicMeshComponent.h"
#include "ViewFinderReSettings.h"

UVFDMCompPoolWorldSubsystem::UVFDMCompPoolWorldSubsystem()
{
}

void UVFDMCompPoolWorldSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
    Super::Initialize(Collection);

    auto Setting = GetDefault<UViewFinderReSettings>();
    if (Setting->bUseVFDMCompPool)
        PreparePools(Setting->CompPoolPrepareNum);
}

void UVFDMCompPoolWorldSubsystem::Deinitialize()
{
    ClearPools();

    Super::Deinitialize();
}

UVFDynamicMeshComponent *UVFDMCompPoolWorldSubsystem::GetOrCreateComp(
    UObject *Outer,
    const TSubclassOf<UVFDynamicMeshComponent> &CompClass)
{
    UVFDynamicMeshComponent *CompRes = nullptr;
    if (!Outer || !CompClass)
        return CompRes;

    if (PoolsOfAvailable.FindOrAdd(CompClass).Comps.Num() > 0)
    {
        CompRes = PoolsOfAvailable[CompClass].Comps.Pop();
        CompRes->Rename(nullptr, Outer);
        return CompRes;
    }

    CompRes = NewObject<UVFDynamicMeshComponent>(Outer, CompClass, NAME_None);
    PoolsOfAll.FindOrAdd(CompClass).Comps.Add(CompRes);

    ensureMsgf(!CompRes->IsEnabled(),
               TEXT("%s must get comp not enabled."),
               __FUNCTIONW__);
    return CompRes;
}

bool UVFDMCompPoolWorldSubsystem::ReturnComp(UVFDynamicMeshComponent *Comp)
{
    auto CompClass = Comp->GetClass();
    if (!PoolsOfAll.Contains(CompClass) || !PoolsOfAll[CompClass].Comps.Contains(Comp))
        return false;

    ensureMsgf(!Comp->IsEnabled(),
               TEXT("%s must return comp not enabled."),
               __FUNCTIONW__);

    Comp->Rename(nullptr, this);
    PoolsOfAvailable.FindOrAdd(CompClass).Comps.AddUnique(Comp);
    return true;
}

void UVFDMCompPoolWorldSubsystem::PreparePools(const TMap<TSubclassOf<UVFDynamicMeshComponent>, int> &PrepareNum)
{
    for (auto [CompClass, Num] : PrepareNum)
    {
        PoolsOfAll.FindOrAdd(CompClass).Comps.Reset(Num);
        PoolsOfAvailable.FindOrAdd(CompClass).Comps.Reset(Num);

        for (int i = 0; i < Num; i++)
        {
            UVFDynamicMeshComponent *Comp = NewObject<UVFDynamicMeshComponent>(
                this, CompClass, NAME_None);
            PoolsOfAll[CompClass].Comps.Emplace(Comp);
            PoolsOfAvailable[CompClass].Comps.Emplace(Comp);
        }
    }
}

void UVFDMCompPoolWorldSubsystem::ClearPools(bool bForceGarbage)
{
    PoolsOfAll.Reset();
    PoolsOfAvailable.Reset();
    if (bForceGarbage)
        GEngine->ForceGarbageCollection(true);
}