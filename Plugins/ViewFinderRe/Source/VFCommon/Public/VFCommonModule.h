// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/*
通用模块: 简化日志Channel使用, 统一的UDeveloperSettings位置
*/
class FVFCommonModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

#if WITH_EDITOR
public:
	void ReadPluginInfos();

	void AddPropertySections();
#endif
};