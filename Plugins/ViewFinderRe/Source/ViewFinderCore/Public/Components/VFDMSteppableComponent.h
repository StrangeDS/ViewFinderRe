#pragma once

#include "CoreMinimal.h"
#include "VFDynamicMeshComponent.h"
#include "VFStepsRecordInterface.h"
#include "VFDMSteppableComponent.generated.h"

class VFStepsRecordInterface;
class UVFStepsRecorderWorldSubsystem;

UENUM(BlueprintType)
enum class UVFDMCompStepOperation
{
	None = 0,
	BeginPlay,
	CopyMeshFromComponent,
	RegisterToTransformRecorder,
	ReplaceMeshForComponent,
	// 交集, 差集, 并集需要保存网格体, 故在本地处理
	IntersectMeshWithDMComp,
	SubtractMeshWithDMComp,
	UnionMeshWithDMComp
};

USTRUCT(BlueprintType)
struct FVFDMCompStep
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	UVFDMCompStepOperation Operation = UVFDMCompStepOperation::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<UDynamicMesh> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float Time = 0.f;
};

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API UVFDMSteppableComponent : public UVFDynamicMeshComponent, public IVFStepsRecordInterface
{
	GENERATED_BODY()

public:
	UVFDMSteppableComponent(const FObjectInitializer &ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void DestroyComponent(bool bPromoteChildren = false) override;

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	UDynamicMesh *RequestACopiedMesh();

public:
	virtual void CopyMeshFromComponent(UPrimitiveComponent *Source) override;

	virtual void ReplaceMeshForComponent(UPrimitiveComponent *Source) override;

	virtual void IntersectMeshWithDMComp(UDynamicMeshComponent *Tool) override;

	virtual void SubtractMeshWithDMComp(UDynamicMeshComponent *Tool) override;

	virtual void UnionMeshWithDMComp(UDynamicMeshComponent *Tool) override;

public:
	virtual void TickBackward_Implementation(float Time) override;

	UPROPERTY(BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UVFStepsRecorderWorldSubsystem> StepRecorder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TArray<FVFDMCompStep> Steps;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UDynamicMeshPool> LocalPool;
};
