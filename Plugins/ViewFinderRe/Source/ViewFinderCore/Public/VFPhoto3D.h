#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFPhoto3D.generated.h"

UENUM(BlueprintType)
enum class EVFPhoto3DState : uint8
{
	None,
	FirstFold,
	Folded,
	Placed
};

class UVFDynamicMeshComponent;

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPhoto3D : public AActor
{
	GENERATED_BODY()

public:
	AVFPhoto3D();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

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
	TSubclassOf<class UVFDynamicMeshComponent> VFDMCompClass;

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
		const TArray<TEnumAsByte<EObjectTypeQuery>> &ObjectTypes);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<UVFViewFrustumComponent> ViewFrustumRecorder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	bool bOnlyOverlapWithHelps = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesToOverlap;
};
