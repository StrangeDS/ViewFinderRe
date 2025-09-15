#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VFPoolableInterface.generated.h"

UINTERFACE(MinimalAPI)
class UVFPoolableInterface : public UInterface
{
	GENERATED_BODY()
};

// UVFUObjsPoolWorldSubsystem接受的类型接口
class VFUOBJSPOOL_API IVFPoolableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void AfterGet();
	virtual void AfterGet_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void BeforeReturn();
	virtual void BeforeReturn_Implementation();
};
