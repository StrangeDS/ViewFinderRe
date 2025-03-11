// Copyright Epic Games, Inc. All Rights Reserved.

#include "ViewFinderCore.h"

#define LOCTEXT_NAMESPACE "FViewFinderCoreModule"

void FViewFinderCoreModule::StartupModule()
{
#if WITH_EDITOR
	FPropertyEditorModule &PropertyModule =
		FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	TSharedRef<FPropertySection> Section = PropertyModule.FindOrCreateSection(
		"Object",
		"ViewFinderRe",
		LOCTEXT("SectionName", "ViewFinderRe"));
	Section->AddCategory("ViewFinder");
#endif
}

void FViewFinderCoreModule::ShutdownModule()
{
#if WITH_EDITOR
	// AI: 
	// FindOrCreateSection() 的设计初衷是“按需创建，自动管理”，手动移除违背了这一设计哲学。
	// Section 的生命周期由引擎的 FPropertyEditorModule 管理，模块卸载时会自动清理相关资源。
	// 手动移除可能导致意外行为（如重复释放或模块依赖问题）
	FPropertyEditorModule &PropertyModule =
		FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.RemoveSection("Object", "ViewFinder");
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FViewFinderCoreModule, ViewFinderCore)