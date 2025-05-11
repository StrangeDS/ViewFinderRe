#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VFActivatableInterface.generated.h"

UINTERFACE(MinimalAPI)
class UVFActivatableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * (动态装卸的)设备接口
 */
class VIEWFINDERRE_API IVFActivatableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void Activate();
	virtual void Activate_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void Deactivate();
	virtual void Deactivate_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	bool TryActivate();
	virtual bool TryActivate_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	bool TryDeactivate();
	virtual bool TryDeactivate_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	bool CanActivate();
	virtual bool CanActivate_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	bool IsActive();
	virtual bool IsActive_Implementation();
};