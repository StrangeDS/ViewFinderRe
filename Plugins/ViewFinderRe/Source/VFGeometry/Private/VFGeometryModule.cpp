// Copyright StrangeDS. All Rights Reserved.

#include "VFGeometryModule.h"

#include "Modules/ModuleManager.h"

#if !WITH_EDITOR
#include "VFGeometryDeveloperSettings.h"
#endif

#define LOCTEXT_NAMESPACE "FVFGeometryModule"

void FVFGeometryModule::StartupModule()
{
}

void FVFGeometryModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVFGeometryModule, VFGeometry)