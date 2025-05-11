#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VFHelperComponent.h"
#include "VFHelperInterface.generated.h"

UINTERFACE(MinimalAPI)
class UVFHelperInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 携带Helper组件
 */
class VIEWFINDERCORE_API IVFHelperInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	UVFHelperComponent *GetHelper();
	virtual UVFHelperComponent *GetHelper_Implementation();
};
