// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

// Editor utility class for PhotoCatcher.
class FVFPhotoCatcherEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
};