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

	virtual void OnConstruction(const FTransform &Transform) override;

	virtual void BeginPlay() override;

	virtual TArray<UPrimitiveComponent *> GetOverlapComps_Implementation() override;

	void HideCurLevel();

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bOnlyCollectInSameLevel = true;
#endif

#if WITH_EDITOR
public:
	UFUNCTION(CallInEditor, Category = "ViewFinder")
	void CollectCompsInLevels();

	UFUNCTION(CallInEditor, Category = "ViewFinder")
	void CollectCompsInSameLevel();

	UFUNCTION(CallInEditor, Category = "ViewFinder")
	void ClearCompsInEditor();

	UFUNCTION(CallInEditor, Category = "ViewFinder")
	void UpdateOnlyActorsCatched();
#endif

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TObjectPtr<UPrimitiveComponent>> CompsInEditor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TObjectPtr<AActor>> OnlyActorsCatched;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder", meta = (MakeEditWidget))
	FTransform PhotoSpawnPoint = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	bool bHideChildActors = true;
};
