// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/DynamicMeshComponent.h"
#include "VFGeometryFunctions.h"
#include "VFViewFrustumComponent.generated.h"

UCLASS(Blueprintable, ClassGroup = (ViewFinder), meta = (BlueprintSpawnableComponent))
class VFGEOMETRY_API UVFViewFrustumComponent : public UDynamicMeshComponent
{
	GENERATED_BODY()

	UVFViewFrustumComponent();

public:
	void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void GenerateViewFrustum(float Angle, float AspectRatio, float StartDis, float EndDis);
	virtual void GenerateViewFrustum_Implementation(float Angle, float AspectRatio, float StartDis, float EndDis);

	// Needs to be called to update in Actor's construction
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void RegenerateViewFrustum(float Angle = 90.f, float AspectRatio = 1.77778f, float StartDis = 10.0f, float EndDis = 500.0f);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void RecordViewFrustum(UVFViewFrustumComponent *Other);

#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	void DrawConvexCollision();
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<UMaterialInterface> Material;
};
