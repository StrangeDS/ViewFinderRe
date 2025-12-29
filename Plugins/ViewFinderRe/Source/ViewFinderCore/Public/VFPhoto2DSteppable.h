// Copyright 2026, StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VFPhoto2D.h"
#include "VFStepsRecordInterface.h"
#include "VFPhoto2DSteppable.generated.h"

UENUM(BlueprintType)
enum class EVFPhoto2DSteppableOperation : uint8
{
	None,
	Folded,
	Placed,
	ReattachTo,
	MAX
};

USTRUCT(BlueprintType)
struct FVFPhoto2DHierarchyInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	USceneComponent *AttachParent;

	// Scaling is not managed by this and will be ignored.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	FTransform RelativeTramsform;
};

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPhoto2DSteppable : public AVFPhoto2D, public IVFStepsRecordInterface
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	virtual void FoldUp() override;

	virtual void PlaceDown() override;

	virtual bool ReattachToComponent(USceneComponent *Target) override;

public:
	virtual bool StepBack_Implementation(FVFStepInfo &StepInfo) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TArray<FVFPhoto2DHierarchyInfo> HierarchyRecorder;
};