#pragma once

#include "CoreMinimal.h"
#include "Components/DynamicMeshComponent.h"
#include "VFGeometryFunctions.h"
#include "VFViewFrustumComponent.generated.h"

UCLASS(Blueprintable, ClassGroup = (ViewFinder), meta = (BlueprintSpawnableComponent))
class VIEWFINDERCORE_API UVFViewFrustumComponent : public UDynamicMeshComponent
{
	GENERATED_BODY()

	UVFViewFrustumComponent();

public:
	void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void GenerateViewFrustum(float Angle, float AspectRatio, float StartDis, float EndDis);
	virtual void GenerateViewFrustum_Implementation(float Angle, float AspectRatio, float StartDis, float EndDis);

	// 需要在Actor的construction中调用
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void RegenerateViewFrustum(float Angle = 90.f, float AspectRatio = 1.77778f, float StartDis = 10.0f, float EndDis = 500.0f);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void RecordViewFrustum(UVFViewFrustumComponent *Other);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	FVF_GeometryScriptPrimitiveOptions PrimitiveOptions{
		EVF_GeometryScriptPrimitivePolygroupMode::SingleGroup};

	// 重点在于: 凸包生成, 面数, 不要识别为基础形状.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	FVF_GeometryScriptCollisionFromMeshOptions CollisionOptions{
		false,
		EVF_GeometryScriptCollisionGenerationMethod::ConvexHulls,
		false,
		false,
		false,
		1.0,
		false,
		6,
		1,
		.5f,
		0,
		0.1f,
		0.1f,
		EVF_GeometryScriptSweptHullAxis::Z,
		true,
		0};

#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	void DrawConvexCollision();
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<UMaterialInterface> Matirial;
};
