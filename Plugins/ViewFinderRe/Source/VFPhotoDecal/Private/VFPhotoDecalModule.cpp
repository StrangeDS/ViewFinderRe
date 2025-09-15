// Copyright StrangeDS. All Rights Reserved.

#include "VFPhotoDecalModule.h"

#define LOCTEXT_NAMESPACE "FVFPhotoDecalModule"

void FVFPhotoDecalModule::StartupModule()
{
#if WITH_EDITOR
    CheckVSM();
#endif
}

void FVFPhotoDecalModule::ShutdownModule()
{
}

#if WITH_EDITOR
void FVFPhotoDecalModule::CheckVSM()
{
	const FEngineVersion &EngineVer = FEngineVersion::Current();

	/*
	场景捕捉与VSM存在冲突(<5.4), 捕获到的阴影存在缺失.
	https://dev.epicgames.com/documentation/en-us/unreal-engine/virtual-shadow-maps-in-unreal-engine#scene-capture
	*/
	if (EngineVer.GetMajor() == 5 && EngineVer.GetMinor() < 4)
	{
		IConsoleVariable *CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.Virtual.Enable"));
		if (CVar && CVar->GetInt() != 0)
		{
			ensureMsgf(false,
					   TEXT("SceneCapture2d has problem with VSM enabled. "
							"Disable VSM or use engine with 5.4(or a higher version)."));
		}
	}
}
#endif

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVFPhotoDecalModule, VFPhotoDecal)