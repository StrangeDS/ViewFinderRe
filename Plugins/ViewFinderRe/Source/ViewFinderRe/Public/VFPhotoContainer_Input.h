#pragma once

#include "CoreMinimal.h"
#include "VFPhotoContainerSteppable.h"
#include "VFPhotoContainer_Input.generated.h"

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERRE_API AVFPhotoContainer_Input : public AVFPhotoContainerSteppable
{
	GENERATED_BODY()

public:
	AVFPhotoContainer_Input();

	virtual void SetEnabled(const bool &Enabled) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UInputMappingContext> MappingContext;
};
