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
class VIEWFINDERCORE_API AVFPhoto3D : public AActor
{
	GENERATED_BODY()

public:
	AVFPhoto3D();

public:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	virtual void FoldUp();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	virtual void PlaceDown();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	void SetViewFrustumVisible(const bool &Visiblity);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	void SetVFDMCompsEnabled(const bool &Enabled);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder|ClassSetting")
	TSubclassOf<UVFDynamicMeshComponent> VFDMCompClass;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	EVFPhoto3DState State = EVFPhoto3DState::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

public: // 记录属性
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void RecordProperty(
		UVFViewFrustumComponent *ViewFrustum,
		bool OnlyWithHelps,
		const TArray<TEnumAsByte<EObjectTypeQuery>> &ObjectTypes,
		EVFPhoto3DState StateIn = EVFPhoto3DState::None);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<UVFViewFrustumComponent> ViewFrustumRecorder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	bool bOnlyOverlapWithHelps = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesToOverlap;

	// 一般情况下都为空
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TObjectPtr<AActor>> ActorsToIgnore;
};
