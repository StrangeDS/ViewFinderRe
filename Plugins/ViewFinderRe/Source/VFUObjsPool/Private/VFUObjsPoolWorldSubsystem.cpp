// Copyright 2026, StrangeDS. All Rights Reserved.

#include "VFUObjsPoolWorldSubsystem.h"

#include "Engine/Engine.h"

#include "VFPoolableInterface.h"

UVFUObjsPoolWorldSubsystem::UVFUObjsPoolWorldSubsystem()
{
}

void UVFUObjsPoolWorldSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
    Super::Initialize(Collection);
}

void UVFUObjsPoolWorldSubsystem::Deinitialize()
{
    ClearPools();

    Super::Deinitialize();
}

UObject *UVFUObjsPoolWorldSubsystem::GetOrCreateAsUObject(
    UObject *Outer,
    const TSubclassOf<UObject> &ObjClass)
{
    check(Outer && ObjClass);

    UObject *ObjRes = nullptr;
    if (PoolsOfAvailable.FindOrAdd(ObjClass).Objs.Num() > 0)
    {
        ObjRes = PoolsOfAvailable[ObjClass].Objs.Pop().GetObject();
        ObjRes->Rename(nullptr, Outer);
        return ObjRes;
    }

    ObjRes = NewObject<UObject>(Outer, ObjClass, NAME_None);
    PoolsOfAll.FindOrAdd(ObjClass).Objs.Add(ObjRes);

    IVFPoolableInterface::Execute_AfterGet(ObjRes);
    return ObjRes;
}

bool UVFUObjsPoolWorldSubsystem::Return(UObject *Obj)
{
    check(Obj);

    auto ObjClass = Obj->GetClass();
    if (!PoolsOfAll.Contains(ObjClass) || !PoolsOfAll[ObjClass].Objs.Contains(Obj))
        return false;

    IVFPoolableInterface::Execute_BeforeReturn(Obj);
    Obj->Rename(nullptr, this);
    PoolsOfAvailable.FindOrAdd(ObjClass).Objs.AddUnique(Obj);
    return true;
}

void UVFUObjsPoolWorldSubsystem::PreparePools(const TMap<TSubclassOf<UObject>, int> &PrepareNum)
{
    for (auto [ObjClass, Num] : PrepareNum)
    {
        PoolsOfAll.FindOrAdd(ObjClass).Objs.Reset(Num);
        PoolsOfAvailable.FindOrAdd(ObjClass).Objs.Reset(Num);

        for (int i = 0; i < Num; i++)
        {
            UObject *Obj = NewObject<UObject>(
                this, ObjClass, NAME_None);
            PoolsOfAll[ObjClass].Objs.Emplace(Obj);
            PoolsOfAvailable[ObjClass].Objs.Emplace(Obj);
        }
    }
}

void UVFUObjsPoolWorldSubsystem::ClearPools(bool bForceGarbage)
{
    PoolsOfAll.Reset();
    PoolsOfAvailable.Reset();
    GEngine->ForceGarbageCollection(bForceGarbage);
}