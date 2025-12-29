// Copyright 2026, StrangeDS. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

// Geometry Strategy Implementation Module: Using the GeometryScript Plugin
class FVFGSGeometryScriptModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
};