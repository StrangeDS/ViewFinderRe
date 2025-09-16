// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// 交互及相关模块
class FVFInteractModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
};