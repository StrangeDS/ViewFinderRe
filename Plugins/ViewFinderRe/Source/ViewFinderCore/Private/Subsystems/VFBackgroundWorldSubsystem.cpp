#include "Subsystems/VFBackgroundWorldSubsystem.h"
#include "VFBackgroundWorldSubsystem.h"

bool UVFBackgroundWorldSubsystem::Register(AActor *Background)
{
    check(IsValid(Background));

    if (Backgrounds.Contains(Background))
        return false;

    Backgrounds.Add(Background);
    return true;
}

bool UVFBackgroundWorldSubsystem::Unregister(AActor *Background)
{
    check(IsValid(Background));

    if (!Backgrounds.Contains(Background))
        return false;

    Backgrounds.RemoveSwap(Background);
    return true;
}

TArray<AActor *> UVFBackgroundWorldSubsystem::GetBackgrounds()
{
    return Backgrounds;
}