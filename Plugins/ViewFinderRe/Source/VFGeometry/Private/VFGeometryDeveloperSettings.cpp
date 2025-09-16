#include "VFGeometryDeveloperSettings.h"

#include "VFGSNone.h"

UVFGeometryDeveloperSettings::UVFGeometryDeveloperSettings(
    const FObjectInitializer &ObjectInitializer)
{
    GeometryStrategyClass = UVFGSNone::StaticClass();
}

#if WITH_EDITOR
void UVFGeometryDeveloperSettings::PostEditChangeProperty(
    FPropertyChangedEvent &PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (!IsValid(GeometryStrategyClass))
    {
        GeometryStrategyClass = UVFGSNone::StaticClass();
        Modify();
        SaveConfig();
    }
}
#endif

bool UVFGeometryDeveloperSettings::IsGeometryStrategyNone()
{
    auto Settings = GetDefault<UVFGeometryDeveloperSettings>();
    if (!IsValid(Settings->GeometryStrategyClass))
        return true;

    return Settings->GeometryStrategyClass == UVFGSNone::StaticClass();
}