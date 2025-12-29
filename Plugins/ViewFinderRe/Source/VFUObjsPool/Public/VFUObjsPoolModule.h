// Copyright 2026, StrangeDS. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

// UObject pool module, requiring implementation of VFPoolableInterface
class FVFUObjsPoolModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
};