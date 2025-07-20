#include "ViewFinderReSettings.h"

UViewFinderReSettings::UViewFinderReSettings(const FObjectInitializer &ObjectInitializer)
{
}

FName UViewFinderReSettings::GetContainerName() const
{
    return FName(TEXT("Project"));
}

FName UViewFinderReSettings::GetCategoryName() const
{
    return FName(TEXT("Plugins"));
}

FName UViewFinderReSettings::GetSectionName() const
{
    return FName(TEXT("UViewFinderReSettings"));
}

UViewFinderReSettings *UViewFinderReSettings::Get()
{
    return GetMutableDefault<UViewFinderReSettings>();
}

void UViewFinderReSettings::Save(UViewFinderReSettings *Setting)
{
    if (!Setting)
        Setting = Get();

    Setting->Modify();
    Setting->SaveConfig();
}