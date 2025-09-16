// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// 步骤记录模块
class FVFStepsRecorderModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
};