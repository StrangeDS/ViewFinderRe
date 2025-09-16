// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FViewFinderReModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
};
