#include "VFUObjsRegistarWorldSubsystem.h"

UVFUObjsRegistarWorldSubsystem::UVFUObjsRegistarWorldSubsystem()
{
}

void UVFUObjsRegistarWorldSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
    Super::Initialize(Collection);
}

void UVFUObjsRegistarWorldSubsystem::Deinitialize()
{
    ClearAll();

    Super::Deinitialize();
}

bool UVFUObjsRegistarWorldSubsystem::Register(UObject *Obj,
                                              const FString &Channel,
                                              TSubclassOf<UObject> ObjClass)
{
    check(IsValid(Obj));

    if (!ObjClass)
        ObjClass = Obj->GetClass();

    auto &Table = ChannelsRegisted.FindOrAdd(Channel).Table;
    auto &Objs = Table.FindOrAdd(ObjClass).Objs;
    if (Objs.Contains(Obj))
        return false;

    Objs.Emplace(Obj);
    return true;
}

bool UVFUObjsRegistarWorldSubsystem::Unregister(UObject *Obj,
                                                const FString &Channel,
                                                TSubclassOf<UObject> ObjClass)
{
    // 也理应在生命周期结束前反注册
    check(IsValid(Obj));

    if (!ObjClass)
        ObjClass = Obj->GetClass();

    if (!ChannelsRegisted.Contains(Channel))
        return false;

    auto &Table = ChannelsRegisted[Channel].Table;
    if (!Table.Contains(ObjClass))
        return false;

    Table[ObjClass].Objs.RemoveSwap(Obj);
    return true;
}

void UVFUObjsRegistarWorldSubsystem::ClearInvalidInChannel(
    const FString &Channel,
    bool bShrink)
{
    if (!ChannelsRegisted.Contains(Channel))
        return;

    for (auto &[ObjClass, UObjs] : ChannelsRegisted[Channel].Table)
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

void UVFUObjsRegistarWorldSubsystem::ClearAllInChannel(
    const FString &Channel,
    bool bShrink)
{
    if (!ChannelsRegisted.Contains(Channel))
        return;

    auto &Table = ChannelsRegisted[Channel].Table;
    if (bShrink)
        Table.Empty();
    else
        Table.Reset();
}

TArray<UObject *> UVFUObjsRegistarWorldSubsystem::K2_GetUObjs(
    const FString &Channel,
    TSubclassOf<UObject> ObjClass)
{
    return GetUObjs<UObject>(Channel, ObjClass);
}

void UVFUObjsRegistarWorldSubsystem::ClearAll(bool bShrink)
{
    ChannelsRegisted.Reset();
}