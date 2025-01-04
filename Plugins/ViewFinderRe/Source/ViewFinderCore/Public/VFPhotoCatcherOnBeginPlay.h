#pragma once

#include "CoreMinimal.h"
#include "VFPhotoCatcher.h"

#include "TimerManager.h"

#include "VFPhotoCatcherOnBeginPlay.generated.h"

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPhotoCatcherOnBeginPlay : public AVFPhotoCatcher
{
	GENERATED_BODY()
	
public:
	AVFPhotoCatcherOnBeginPlay();

	virtual void BeginPlay() override;
	
public:
	// 世界Transform
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder", meta = (MakeEditWidget))
	FTransform PhotoSpawnPoint = FTransform::Identity;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	bool bOnlyCatchChildActors = false;
};
