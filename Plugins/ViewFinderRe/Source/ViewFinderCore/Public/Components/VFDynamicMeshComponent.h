#pragma once

#include "CoreMinimal.h"
#include "Components/DynamicMeshComponent.h"
#include "VFDynamicMeshComponent.generated.h"

UCLASS(Blueprintable, ClassGroup = (ViewFinder), meta = (BlueprintSpawnableComponent))
class VIEWFINDERCORE_API UVFDynamicMeshComponent : public UDynamicMeshComponent
{
	GENERATED_BODY()

public:
	UVFDynamicMeshComponent(const FObjectInitializer &ObjectInitializer = FObjectInitializer::Get());

	virtual void OnRegister() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	virtual void Init(UPrimitiveComponent *Source);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	virtual void Clear();

public: // Mesh
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	virtual void CopyMeshFromComponent(UPrimitiveComponent *Source);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	virtual void ReplaceMeshForComponent(UPrimitiveComponent *Source);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	virtual void IntersectMeshWithDMComp(UDynamicMeshComponent *Tool);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	virtual void SubtractMeshWithDMComp(UDynamicMeshComponent *Tool);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	virtual void UnionMeshWithDMComp(UDynamicMeshComponent *Tool);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void UpdateSimlpeCollision();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void UpdateMaterials();

public: // SourceComponent
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	UPrimitiveComponent *GetSourceComponent();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	UVFDynamicMeshComponent *GetSourceVFDMComp();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void RestoreSourceComponent();

#if WITH_EDITOR
	// 仅用于还原被替换的父组件
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	void RestoreSourceComponentInEditor();

	// 仅用于替换父组件
	UFUNCTION(CallInEditor, Category = "ViewFinder", meta = (DisplayName = "ReplaceMeshForComponent"))
	void ReplaceMeshForComponentInEditor();
#endif

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UPrimitiveComponent> SourceComponent;

public: // Enabled
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void SetEnabled(bool Enabled);

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	bool bEnabled = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	bool bSimulatePhysicsRecorder = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	bool bEnableGravityRecorder = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	bool bCastShadowRecorder = false;
};
