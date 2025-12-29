// Copyright 2026, StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/DynamicMeshComponent.h"
#include "VFPoolableInterface.h"
#include "VFDynamicMeshComponent.generated.h"

// Properties of UVFDynamicMeshComponent that need to be manually recorded
USTRUCT(BlueprintType)
struct FVFDMCompRecordProps
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bSimulatePhysicsRecorder = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bEnableGravityRecorder = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bCastShadowRecorder = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bUseSimpleCollision = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	uint8 LevelOfCollision = 0;

	FVFDMCompRecordProps GenerateNew();

	void Reset();
};

UCLASS(Blueprintable, ClassGroup = (ViewFinder), meta = (BlueprintSpawnableComponent))
class VFGEOMETRY_API UVFDynamicMeshComponent
	: public UDynamicMeshComponent,
	  public IVFPoolableInterface
{
	GENERATED_BODY()

public:
	UVFDynamicMeshComponent(const FObjectInitializer &ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

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
	void UpdateSimpleCollision();

#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	void DrawSimpleShapesCollision();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	void DrawConvexCollision();
#endif

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
	// Only used to restore the replaced parent component in editor
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	void RestoreSourceComponentInEditor();

	// Only used to replace the parent component in editor
	UFUNCTION(CallInEditor, Category = "ViewFinder", meta = (DisplayName = "ReplaceMeshForComponent"))
	void ReplaceMeshForComponentInEditor();
#endif

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UPrimitiveComponent> SourceComponent;

public: // Enabled
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	virtual void SetEnabled(bool Enabled);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	bool IsEnabled();

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder")
	bool bEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	FVFDMCompRecordProps Props;

public: // IVFPoolableInterface
	virtual void AfterGet_Implementation();

	virtual void BeforeReturn_Implementation();
};