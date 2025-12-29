// Copyright 2026, StrangeDS. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

// A World Subsystem module for registering UObject-derived classes
class FVFUObjsRegistrarModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
};