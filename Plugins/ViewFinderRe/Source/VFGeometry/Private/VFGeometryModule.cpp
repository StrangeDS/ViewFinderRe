// Copyright StrangeDS. All Rights Reserved.

#include "VFGeometryModule.h"

#include "Modules/ModuleManager.h"

#if !WITH_EDITOR
#include "VFGeometryDeveloperSettings.h"
#endif

#define LOCTEXT_NAMESPACE "FVFGeometryModule"

void FVFGeometryModule::StartupModule()
{
    auto &ModuleManager = FModuleManager::Get();

#if WITH_EDITOR
    // 加载可用策略模块
    if (ModuleManager.IsModuleLoaded(TEXT("GeometryScriptingCore")))
    {
        ModuleManager.LoadModuleChecked(TEXT("VFGSGeometryScript"));
    }
    ModuleManager.LoadModuleChecked(TEXT("VFGSGeometryScriptNative"));
#else
    // 加载已选择策略模块
    auto Settings = GetDefault<UVFGeometryDeveloperSettings>();
    if (!Settings->IsGeometryStrategyNone())
    {
        ModuleManager.LoadModuleChecked(
            *Settings->GeometryStrategyClass.Get()->GetName());
    }
#endif
}

void FVFGeometryModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVFGeometryModule, VFGeometry)