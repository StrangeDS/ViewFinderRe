#pragma once

#include "CoreMinimal.h"
#include "Components/DynamicMeshComponent.h"
#include "VFPoolableInterface.h"
#include "VFDynamicMeshComponent.generated.h"

// 动态网格需要手动记录的属性
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
	void UpdateSimlpeCollision();

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