// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VFHelperInterface.generated.h"

class UVFHelperComponent;

UINTERFACE(MinimalAPI)
class UVFHelperInterface : public UInterface
{
	GENERATED_BODY()
};

// Return the Helper component.
class VFPHOTOCOMMON_API IVFHelperInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	UVFHelperComponent *GetHelper();
	virtual UVFHelperComponent *GetHelper_Implementation();
};