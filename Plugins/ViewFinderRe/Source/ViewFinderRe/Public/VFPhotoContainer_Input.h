// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VFPhotoContainerSteppable.h"
#include "VFActivatableInterface.h"
#include "VFPhotoContainer_Input.generated.h"

class UInputMappingContext;

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERRE_API AVFPhotoContainer_Input : public AVFPhotoContainerSteppable,
												 public IVFActivatableInterface
{
	GENERATED_BODY()

public:
	AVFPhotoContainer_Input();

	virtual void SetEnabled(const bool &Enabled) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UInputMappingContext> MappingContext;

public: // IVFActivatableInterface
	virtual void Activate_Implementation() override;

	virtual void Deactivate_Implementation() override;

	virtual bool CanActivate_Implementation() override;

	virtual bool IsActive_Implementation() override;
};