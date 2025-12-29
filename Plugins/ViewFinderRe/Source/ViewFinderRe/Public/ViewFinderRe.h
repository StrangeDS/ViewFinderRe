// Copyright 2026, StrangeDS. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FViewFinderReModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
};