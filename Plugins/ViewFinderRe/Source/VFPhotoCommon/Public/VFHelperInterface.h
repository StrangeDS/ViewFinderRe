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

/*
接口: 携带Helper组件, 并返回Helper组件
*/
class VFPHOTOCOMMON_API IVFHelperInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	UVFHelperComponent *GetHelper();
	virtual UVFHelperComponent *GetHelper_Implementation();
};
