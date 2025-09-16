// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// 用于UObject衍生类的注册世界子系统模块
class FVFUObjsRegistarModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
};