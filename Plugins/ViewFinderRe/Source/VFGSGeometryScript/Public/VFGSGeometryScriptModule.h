// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// 几何策略实现模块: 使用GeometryScript插件
class FVFGSGeometryScriptModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
};