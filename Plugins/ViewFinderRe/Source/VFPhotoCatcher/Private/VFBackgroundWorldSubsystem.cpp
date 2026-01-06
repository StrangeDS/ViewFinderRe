// Copyright 2026, StrangeDS. All Rights Reserved.

#include "VFBackgroundWorldSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"

#include "VFUObjsRegistrarWorldSubsystem.h"

bool UVFBackgroundWorldSubsystem::Register(AActor *Background)
{
    check(IsValid(Background));
    auto Registrar = GetWorld()->GetSubsystem<UVFUObjsRegistrarWorldSubsystem>();
    return Registrar->Register(Background, ChannelName, AActor::StaticClass());
}

bool UVFBackgroundWorldSubsystem::Unregister(AActor *Background)
{
    check(IsValid(Background));
    auto Registrar = GetWorld()->GetSubsystem<UVFUObjsRegistrarWorldSubsystem>();
    return Registrar->Unregister(Background, ChannelName, AActor::StaticClass());
}

TArray<AActor *> UVFBackgroundWorldSubsystem::GetBackgrounds()
{
    auto Registrar = GetWorld()->GetSubsystem<UVFUObjsRegistrarWorldSubsystem>();
    return Registrar->GetUObjs<AActor>(ChannelName, AActor::StaticClass());
}