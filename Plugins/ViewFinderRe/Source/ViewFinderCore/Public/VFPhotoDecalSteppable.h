#pragma once

#include "CoreMinimal.h"
#include "VFPhotoDecal.h"
#include "VFStepsRecordInterface.h"
#include "VFPhotoDecalSteppable.generated.h"

UENUM(BlueprintType)
enum class AVFPhotoDecalOperation : uint8
{
	None = 0,
	Replace,
	Restore
};

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPhotoDecalSteppable : public AVFPhotoDecal, public IVFStepsRecordInterface
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	virtual void ReplaceWithDecal_Implementation(bool ForceToUpdate = false) override;

	virtual void RestoreWithActors_Implementation() override;

public:
	virtual bool StepBack_Implementation(FVFStepInfo &StepInfo) override;
};