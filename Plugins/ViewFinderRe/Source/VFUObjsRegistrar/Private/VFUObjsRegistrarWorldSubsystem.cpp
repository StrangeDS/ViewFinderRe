// Copyright StrangeDS. All Rights Reserved.

#include "VFUObjsRegistrarWorldSubsystem.h"

UVFUObjsRegistrarWorldSubsystem::UVFUObjsRegistrarWorldSubsystem()
{
}

void UVFUObjsRegistrarWorldSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
    Super::Initialize(Collection);
}

void UVFUObjsRegistrarWorldSubsystem::Deinitialize()
{
    ClearAll();

    Super::Deinitialize();
}

bool UVFUObjsRegistrarWorldSubsystem::Register(UObject *Obj,
                                              const FString &Channel,
                                              TSubclassOf<UObject> ObjClass)
{
    check(IsValid(Obj));

    if (!ObjClass)
        ObjClass = Obj->GetClass();

    auto &Table = ChannelsRegistered.FindOrAdd(Channel).Table;
    auto &Objs = Table.FindOrAdd(ObjClass).Objs;
    if (Objs.Contains(Obj))
        return false;

    Objs.Emplace(Obj);
    return true;
}

bool UVFUObjsRegistrarWorldSubsystem::Unregister(UObject *Obj,
                                                const FString &Channel,
                                                TSubclassOf<UObject> ObjClass)
{
    // Should unregister before the lifecycle ends
    check(IsValid(Obj));

    if (!ObjClass)
        ObjClass = Obj->GetClass();

    if (!ChannelsRegistered.Contains(Channel))
        return false;

    auto &Table = ChannelsRegistered[Channel].Table;
    if (!Table.Contains(ObjClass))
        return false;

    Table[ObjClass].Objs.RemoveSwap(Obj);
    return true;
}

void UVFUObjsRegistrarWorldSubsystem::ClearInvalidInChannel(
    const FString &Channel,
    bool bShrink)
{
    if (!ChannelsRegistered.Contains(Channel))
        return;

    for (auto &[ObjClass, UObjs] : ChannelsRegistered[Channel].Table)
    {
        auto &Objs = UObjs.Objs;
        for (auto It = Objs.CreateIterator(); It; ++It)
        {
            if (!IsValid(*It))
                It.RemoveCurrent();
        }
        if (bShrink)
            Objs.Shrink();
    }
}

void UVFUObjsRegistrarWorldSubsystem::ClearAllInChannel(
    const FString &Channel,
    bool bShrink)
{
    if (!ChannelsRegistered.Contains(Channel))
        return;

    auto &Table = ChannelsRegistered[Channel].Table;
    if (bShrink)
        Table.Empty();
    else
        Table.Reset();
}

TArray<UObject *> UVFUObjsRegistrarWorldSubsystem::K2_GetUObjs(
    const FString &Channel,
    TSubclassOf<UObject> ObjClass)
{
    return GetUObjs<UObject>(Channel, ObjClass);
}

void UVFUObjsRegistrarWorldSubsystem::ClearAll(bool bShrink)
{
    ChannelsRegistered.Reset();
}