// Copyright 2026, StrangeDS. All Rights Reserved.

#include "VFCommonModule.h"

#if WITH_EDITOR
#include "Interfaces/IPluginManager.h"
#include "VFInfoDeveloperSettings.h"
#endif

#define LOCTEXT_NAMESPACE "FVFCommonModule"

void FVFCommonModule::StartupModule()
{
#if WITH_EDITOR
	ReadPluginInfos();
	AddPropertySections();
#endif
}

void FVFCommonModule::ShutdownModule()
{
}

#if WITH_EDITOR
void FVFCommonModule::ReadPluginInfos()
{
	IPluginManager &PluginManager = IPluginManager::Get();
	TSharedPtr<IPlugin> ViewFinderRe = PluginManager.FindPlugin(TEXT("ViewFinderRe"));
	if (ViewFinderRe.IsValid())
	{
		FPluginDescriptor PluginDescriptor = ViewFinderRe->GetDescriptor();
		auto Settings = GetMutableDefault<UVFInfoDeveloperSettings>();
		Settings->Description = PluginDescriptor.Description;
		Settings->DocsURL = PluginDescriptor.DocsURL;
		Settings->Developer = PluginDescriptor.CreatedBy;
	}
}

void FVFCommonModule::AddPropertySections()
{
	FPropertyEditorModule &PropertyModule =
		FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	TSharedRef<FPropertySection> Section = PropertyModule.FindOrCreateSection(
		"Object",
		"ViewFinderRe",
		LOCTEXT("SectionName", "ViewFinderRe"));
	Section->AddCategory("ViewFinder");
}
#endif

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVFCommonModule, VFCommon)