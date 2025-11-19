// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFPhoto3D.generated.h"

class UVFDynamicMeshComponent;

UENUM(BlueprintType)
enum class EVFPhoto3DState : uint8
{
	None,
	FirstFold,
	Folded,
	Placed
};

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VFPHOTOCATCHER_API AVFPhoto3D : public AActor
{
	GENERATED_BODY()

public:
	AVFPhoto3D();

public:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext &Context) const override;
#endif

public:
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	virtual void FoldUp();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	virtual void PlaceDown();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	void SetViewFrustumVisible(const bool &Visiblity);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	void SetVFDMCompsEnabled(const bool &Enabled, const bool IncludingActor = true);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	bool bCuttingOthers = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder|ClassSetting")
	TSubclassOf<UVFDynamicMeshComponent> VFDMCompClass;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	EVFPhoto3DState State = EVFPhoto3DState::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UStaticMeshComponent> StaticMesh;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void RecordProperty(
		UVFViewFrustumComponent *ViewFrustum,
		bool OnlyWithHelps,
		const TArray<TEnumAsByte<EObjectTypeQuery>> &ObjectTypes,
		EVFPhoto3DState StateIn = EVFPhoto3DState::None);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UVFViewFrustumComponent> ViewFrustumRecorder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	bool bOnlyOverlapWithHelper = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesToOverlap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TObjectPtr<AActor>> ActorsToIgnore;
};