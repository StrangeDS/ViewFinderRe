// Copyright 2026, StrangeDS. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FVFInteractModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
};