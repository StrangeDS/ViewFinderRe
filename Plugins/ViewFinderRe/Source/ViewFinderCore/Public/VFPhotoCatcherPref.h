#pragma once

#include "CoreMinimal.h"
#include "VFPhotoCatcher.h"
#include "VFPhotoCatcherPref.generated.h"

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPhotoCatcherPref : public AVFPhotoCatcher
{
	GENERATED_BODY()

public:
	AVFPhotoCatcherPref();

	virtual void BeginPlay() override;

	virtual TArray<UPrimitiveComponent *> GetOverlapComps_Implementation() override;

	void HideCurLevel();

#if WITH_EDITOR

	// 你可以用视锥收集后, 再手动删除不需要的Actor. 减少工作量.
	UFUNCTION(CallInEditor, Category = "ViewFinder")
	void RecollectActorsWithFrustum();

#endif

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TObjectPtr<AActor>> OnlyActorsCatched;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder", meta = (MakeEditWidget))
	FTransform PhotoSpawnPoint = FTransform::Identity;
};
