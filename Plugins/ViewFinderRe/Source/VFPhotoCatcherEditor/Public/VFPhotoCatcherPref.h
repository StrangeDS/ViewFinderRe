// Copyright 2026, StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VFPhotoCatcher.h"
#include "VFPhotoCatcherPref.generated.h"

class UMaterialInstanceConstant;

/*
Approaches Attempted:
1. Prepare Actors in the editor, then call TakeAPhoto with a one-frame delay in BeginPlay.
	Solved: Some photo were performed directly in the current ULevel,
	while others required a separate ULevel, then need to hide other Actors in LevelInstance.
	Issue: Difficult to support recursion.
2. Create a new Level Instance from the generated Photo2D and Photo3D in the editor (using ULevelInstanceSubsystem::CreateLevelInstanceFrom()).
	Solved: After moving to the new Level Instance,
	broken references between Photo2D and Photo3D were repaired using GUID pairing.
	Issue: Recursive photo need capture recursively,
	but Material Instance Dynamics (MID) could not be used in the editor.
	Attempted workarounds with two RenderTargets (to avoid read-write conflicts),
	but other processes remained cumbersome and error-prone.
3. (Current approach)Similar to approach 2, but with improvements:
	After refactoring UVFPhotoCaptureComponent,
	it no longer requires Render Targets as a mandatory dependency.
	After TakeAPhoto, new UTexture2D and material instances are directly generated and saved to the Level's directory.
	Since all textures are static, recursive photo capture is possible,
	and new material instances eliminate the need for additional processing in BeginPlay.

Regarding "Recursive Photos":
	1. Create a subclass of Photo2D (e.g., RecursivePhoto2D)?
		Not ideal, as it would require reimplementing the rewind logic.
	2. Hold references?
		Not feasible, as references become useless after moving to a new Level Instance.
	3. Assume all Photo2Ds under Photo3D are recursive?
		Incorrect, as photos within photos could refer to different Photo3Ds.
	4. Identify those without connected Photo3D?
		Promising, but future gameplay might require Photo2D without Photo3D.
	5. (Current approach) Add explicit properties to Photo2D?
		Reasonable, though slightly bloated.

How and When Should Recursive Photo2D Acquire Photo3D?
	1. OnCopyAfterCopiedForPhoto?
		At this point, the external Photo3D is already generated,
		but manually calling Photo3D->RecordProperty() is performance-intensive.
	2. OnCopyAfterPlacedByPhoto?
		The external Photo3D is already placed, but collapsing and duplicating would break backtracking.
		Should duplication occur before collapsing?
	3. (Current approach) Introduce OnCopyBeginPlacedByPhoto.

Unresolved Bug:
Editor crashes when simultaneously deleting a Level Instance and its associated resources (Texture2Ds or MICs).
*/

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VFPHOTOCATCHEREDITOR_API AVFPhotoCatcherPref : public AVFPhotoCatcher
{
	GENERATED_BODY()

public:
	AVFPhotoCatcherPref();

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform &Transform) override;

	virtual TArray<UPrimitiveComponent *> GetOverlapComps_Implementation() override;

	// You can collect with frustum first, then manually delete unwanted Actors to reduce workload.
	UFUNCTION(CallInEditor, Category = "ViewFinder")
	void RecollectShowOnlyActors();

	UFUNCTION(CallInEditor, Category = "ViewFinder")
	void PrefabricateAPhotoLevel();

	/*
	Manual iteration is used to avoid rendering artifacts like
	black screens caused by simultaneous read-write operations,
	and also for version compatibility considerations.
	*/
	UFUNCTION(CallInEditor, Category = "ViewFinder")
	void UpdateMIC();

	virtual AVFPhoto2D *TakeAPhoto_Implementation() override;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "ViewFinder")
	FName TextureName = TEXT("Texture");

	UPROPERTY(EditAnywhere, Category = "ViewFinder")
	int MaterialIndex = 0;

	// If not empty, only the Actors within the list will be processed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TObjectPtr<AActor>> OnlyActorsCatched;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<UMaterialInstanceConstant> MIConstantAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<UTexture2D> Texture2DAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<UMaterialInstanceConstant> BgMIConstantAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<UTexture2D> BgTexture2DAsset;
#endif
#endif
};