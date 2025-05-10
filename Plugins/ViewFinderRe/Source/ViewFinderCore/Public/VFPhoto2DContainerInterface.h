#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VFPhoto2DContainerInterface.generated.h"

class AVFPhoto2D;

UINTERFACE(MinimalAPI)
class UVFPhoto2DContainerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 存放Photo2D的容器
 */
class VIEWFINDERCORE_API IVFPhoto2DContainerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	int GetPhoto2DNum();
	virtual int GetPhoto2DNum_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	bool TakeIn(AVFPhoto2D *Photo2D);
	virtual bool TakeIn_Implementation(AVFPhoto2D *Photo2D, const bool &Enabled = true);
};