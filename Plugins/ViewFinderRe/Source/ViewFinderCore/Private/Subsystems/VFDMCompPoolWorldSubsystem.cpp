#include "VFDMCompPoolWorldSubsystem.h"

#include "UnrealEngine.h"

#include "VFDynamicMeshComponent.h"
#include "VFDMSteppableComponent.h"

UVFDMCompPoolWorldSubsystem::UVFDMCompPoolWorldSubsystem()
{
    PrepareNum = {
        {UVFDynamicMeshComponent::StaticClass(), 100},
        {UVFDMSteppableComponent::StaticClass(), 100},
    };
}

void UVFDMCompPoolWorldSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
    Super::Initialize(Collection);

    PreparePools();
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

    return CompRes;
}

void UVFDMCompPoolWorldSubsystem::ReturnComp(UVFDynamicMeshComponent *Comp)
{
    auto CompClass = Comp->GetClass();
    if (!PoolsOfAll.Contains(CompClass) || !PoolsOfAll[CompClass].Comps.Contains(Comp))
        return;

    Comp->Rename(nullptr, this);
    PoolsOfAvailable.FindOrAdd(CompClass).Comps.AddUnique(Comp);
}

void UVFDMCompPoolWorldSubsystem::PreparePools()
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