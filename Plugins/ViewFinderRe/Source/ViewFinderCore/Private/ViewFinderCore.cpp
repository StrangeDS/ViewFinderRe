// Copyright StrangeDS. All Rights Reserved.

#include "ViewFinderCore.h"

#include "Interfaces/IPluginManager.h"

#include "ViewFinderReSettings.h"

#define LOCTEXT_NAMESPACE "FViewFinderCoreModule"

void FViewFinderCoreModule::StartupModule()
{
#if WITH_EDITOR
	ReadPluginInfos();
	AddPropertySections();
	CheckConfigs();
#endif
}

void FViewFinderCoreModule::ShutdownModule()
{
}

#if WITH_EDITOR
void FViewFinderCoreModule::AddPropertySections()
{
	/*
	AI:
	FindOrCreateSection() 的设计初衷是"按需创建，自动管理"，手动移除违背了这一设计哲学.
	Section 的生命周期由引擎的 FPropertyEditorModule 管理，模块卸载时会自动清理相关资源。
	*/
	FPropertyEditorModule &PropertyModule =
		FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	TSharedRef<FPropertySection> Section = PropertyModule.FindOrCreateSection(
		"Object",
		"ViewFinderRe",
		LOCTEXT("SectionName", "ViewFinderRe"));
	Section->AddCategory("ViewFinder");
}

void FViewFinderCoreModule::CheckConfigs()
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

void FViewFinderCoreModule::ReadPluginInfos()
{
	IPluginManager &PluginManager = IPluginManager::Get();
	TSharedPtr<IPlugin> ViewFinderRe = PluginManager.FindPlugin(TEXT("ViewFinderRe"));
	if (ViewFinderRe.IsValid())
	{
		FPluginDescriptor PluginDescriptor = ViewFinderRe->GetDescriptor();
		if (auto Setting = UViewFinderReSettings::Get(); ensure(Setting))
		{
			Setting->Descriptor.Description = PluginDescriptor.Description;
			Setting->Descriptor.DocsURL = PluginDescriptor.DocsURL;
			Setting->Descriptor.Developer = PluginDescriptor.CreatedBy;
		}
	}
}
#endif

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FViewFinderCoreModule, ViewFinderCore)