// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FVFPhotoDecalModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

#if WITH_EDITOR
	void CheckVSM();
#endif
};
