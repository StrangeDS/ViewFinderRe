// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

// Geometry Strategy Implementation Module: Using the locally ported code of the GeometryScript plugin.
class FVFGSGeometryScriptNativeModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
};