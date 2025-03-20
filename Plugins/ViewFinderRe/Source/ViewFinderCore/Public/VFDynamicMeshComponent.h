#pragma once

#include "CoreMinimal.h"
#include "Components/DynamicMeshComponent.h"

#include "Engine/World.h"

#include "VFDynamicMeshComponent.generated.h"

UCLASS(Blueprintable, ClassGroup = (ViewFinder), meta = (BlueprintSpawnableComponent))
class VIEWFINDERCORE_API UVFDynamicMeshComponent : public UDynamicMeshComponent
{
	GENERATED_BODY()

public:
	UVFDynamicMeshComponent(const FObjectInitializer &ObjectInitializer);

	virtual void BeginPlay() override;

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

public: // SourceComponent
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	UPrimitiveComponent *GetSourceComponent();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	UVFDynamicMeshComponent *GetSourceVFDMComp();

	UPROPERTY(BlueprintReadWrite, Category = "ViewFinder")
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

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UVFDMCompPoolWorldSubsystem> MeshPool;
};
