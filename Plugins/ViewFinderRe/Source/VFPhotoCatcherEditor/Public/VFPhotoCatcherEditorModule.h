// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// PhototCatcher的编辑器工具类
class FVFPhotoCatcherEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
};