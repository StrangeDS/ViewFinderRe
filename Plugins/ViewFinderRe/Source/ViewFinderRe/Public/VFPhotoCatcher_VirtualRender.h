// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VFPhotoCatcher_PickUp.h"
#include "VFPhotoCatcher_VirtualRender.generated.h"

class UMaterialParameterCollection;
class UMaterialParameterCollectionInstance;

/*
Used for reproduce:
the lantern in Outer Wilds Space Station, the Watermelon filter in ViewFinder.
Updating parameters to the material parameter collection.
The post-processing effect is not unloaded during odd-numbered dropdown operations.
*/
UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERRE_API AVFPhotoCatcher_VirtualRender : public AVFPhotoCatcher_PickUp
{
	GENERATED_BODY()

public:
	AVFPhotoCatcher_VirtualRender();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

public:
	virtual void DropDown_Implementation() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UMaterialParameterCollection> MPC;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float RenderRadius = 1000.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UMaterialParameterCollectionInstance> MPCI;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	bool bAddPPAfterDropDown = true;
};