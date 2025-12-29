// Copyright 2026, StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VFActivatableInterface.generated.h"

UINTERFACE(MinimalAPI)
class UVFActivatableInterface : public UInterface
{
	GENERATED_BODY()
};

class VFINTERACT_API IVFActivatableInterface
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