// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VFStandInInterface.generated.h"

UINTERFACE(MinimalAPI)
class UVFStandInInterface : public UInterface
{
	GENERATED_BODY()
};

/*
In ViewFinder, when a Pawn is captured in a photo, it may appear distorted.
The current implementation is as follows:
The Helper defines bReplacedWithStandIn and specifies a stand-in class(which implements the interface) for replacement.
During photo-taking process, a StandIn instance is generated for subsequent processing,
while the original Pawn is ignored, excluded from subsequent processing.
For an example, refer to VFPawnStandIn.
*/
class VFPHOTOCOMMON_API IVFStandInInterface
{
	GENERATED_BODY()

public:
	// Reference: UVFPCommonFunctions::ReplaceWithStandIn()  
	// Use SetOriginalActor_Implementation() to trigger related operations.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void SetOriginalActor(AActor *Original);
	virtual void SetOriginalActor_Implementation(AActor *Original);

	/*
	Direct access in BeginPlay may return null values.
	Triggered accessing is recommended.
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	AActor *GetOriginalActor();
	virtual AActor *GetOriginalActor_Implementation();

	/*
	It is advisable to return a VFDMComp (without an actual mesh),
	so that no corresponding VFDMComp will be generated during subsequent processing.
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	UPrimitiveComponent *GetPrimitiveComp();
	virtual UPrimitiveComponent *GetPrimitiveComp_Implementation();
};