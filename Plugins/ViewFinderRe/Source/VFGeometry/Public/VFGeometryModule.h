// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// 几何模块中间层
class FVFGeometryModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
};