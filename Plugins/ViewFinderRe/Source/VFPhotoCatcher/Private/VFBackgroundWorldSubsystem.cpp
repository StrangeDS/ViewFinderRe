#include "VFBackgroundWorldSubsystem.h"
#include "VFUObjsRegistarWorldSubsystem.h"

bool UVFBackgroundWorldSubsystem::Register(AActor *Background)
{
    check(IsValid(Background));
    auto Registar = GetWorld()->GetSubsystem<UVFUObjsRegistarWorldSubsystem>();
    return Registar->Register(Background, ChannelName, AActor::StaticClass());
}

bool UVFBackgroundWorldSubsystem::Unregister(AActor *Background)
{
    check(IsValid(Background));
    auto Registar = GetWorld()->GetSubsystem<UVFUObjsRegistarWorldSubsystem>();
    return Registar->Unregister(Background, ChannelName, AActor::StaticClass());
}

TArray<AActor *> UVFBackgroundWorldSubsystem::GetBackgrounds()
{
    auto Registar = GetWorld()->GetSubsystem<UVFUObjsRegistarWorldSubsystem>();
    return Registar->GetUObjs<AActor>(ChannelName, AActor::StaticClass());
}