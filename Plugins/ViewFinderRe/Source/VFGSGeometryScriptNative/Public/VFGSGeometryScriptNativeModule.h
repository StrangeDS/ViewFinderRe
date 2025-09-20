// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

// 几何策略实现模块: 使用GeometryScript插件的本地化代码
class FVFGSGeometryScriptNativeModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
};