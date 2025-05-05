#pragma once

#include "CoreMinimal.h"
#include "VFPhoto3D.h"
#include "VFStepsRecordInterface.h"
#include "VFPhoto3DSteppable.generated.h"

class UVFStepsRecorderWorldSubsystem;

// 实际上这里的操作无意义, Photo2DSteppable已经进行了调用. 考虑到以后可能的额外操作, 故还是保留.
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
	
	UPROPERTY(BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UVFStepsRecorderWorldSubsystem> StepRecorder;
};
