#pragma once

#include "CoreMinimal.h"
#include "VFDynamicMeshComponent.h"
#include "VFStepsRecordInterface.h"
#include "VFDMSteppableComponent.generated.h"

class VFStepsRecordInterface;

UENUM(BlueprintType)
enum class UVFDMCompStepOperation
{
	None = 0,
	Init,
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

	virtual void Init(UPrimitiveComponent *Source) override;

	virtual void Clear() override;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TArray<FVFDMCompStep> Steps;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UDynamicMeshPool> LocalPool;
};
