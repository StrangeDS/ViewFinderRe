// Copyright StrangeDS. All Rights Reserved.

#include "VFGeometryBaseModule.h"

#define LOCTEXT_NAMESPACE "FVFGeometryBaseModule"

void FVFGeometryBaseModule::StartupModule()
{
    auto &ModuleManager = FModuleManager::Get();
    if (ModuleManager.IsModuleLoaded("GeometryScriptingCore"))
    {
        ModuleManager.LoadModule("VFGSGeometryScript");
    }
    else
    {
        ModuleManager.OnModulesChanged().AddLambda(
            [&ModuleManager](FName Name, EModuleChangeReason Reason)
            {
				if (Name == "GeometryScriptingCore" && Reason == EModuleChangeReason::ModuleLoaded)
				{
					ModuleManager.LoadModule("VFGSGeometryScript");
				} });
    }
}

void FVFGeometryBaseModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVFGeometryBaseModule, VFGeometryBase)