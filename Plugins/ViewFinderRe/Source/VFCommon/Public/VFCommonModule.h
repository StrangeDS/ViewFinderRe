// Copyright 2026, StrangeDS. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

// Common Module: Simplifies Log Channel usage, provides a unified base class for UDeveloperSettings..
class FVFCommonModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

#if WITH_EDITOR
public:
	void ReadPluginInfos();

	void AddPropertySections();
#endif
};