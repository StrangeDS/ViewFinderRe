// Copyright 2026, StrangeDS. All Rights Reserved.

#include "VFGeometryBaseModule.h"

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FVFGeometryBaseModule"

void FVFGeometryBaseModule::StartupModule()
{
    // Load available strategy modules.
    auto &ModuleManager = FModuleManager::Get();
    if (ModuleManager.IsModuleLoaded(TEXT("GeometryScriptingCore")))
    {
        ModuleManager.LoadModuleChecked(TEXT("VFGSGeometryScript"));
    }
    ModuleManager.LoadModuleChecked(TEXT("VFGSGeometryScriptNative"));
}

void FVFGeometryBaseModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVFGeometryBaseModule, VFGeometryBase)