// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VFPhoto3D.h"
#include "VFStepsRecordInterface.h"
#include "VFPhoto3DSteppable.generated.h"

/*
In practice, the operations here have no practical effect,
as Photo2DSteppable has already handled the calls. Retained for potential future extensions.
*/
UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPhoto3DSteppable : public AVFPhoto3D, public IVFStepsRecordInterface
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	virtual void FoldUp() override;

	virtual void PlaceDown() override;

public:
	virtual bool StepBack_Implementation(FVFStepInfo &StepInfo) override;
};