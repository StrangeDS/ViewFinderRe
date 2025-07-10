#pragma once

#include "CoreMinimal.h"
#include "Components/VFPhotoCaptureComponent.h"
#include "VFBackgroundCaptureComponent.generated.h"

class AVFPlaneActor;

// 专用于背景天空颜色的场景捕捉
UCLASS(Blueprintable, ClassGroup = (ViewFinder), meta = (BlueprintSpawnableComponent))
class VIEWFINDERCORE_API UVFBackgroundCaptureComponent : public UVFPhotoCaptureComponent
{
	GENERATED_BODY()

public:
	UVFBackgroundCaptureComponent();

	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	UPrimitiveComponent *DrawABackgroundWithSize(float Distance, float Width, float Height);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	UPrimitiveComponent *DrawABackground();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TSubclassOf<AVFPlaneActor> PlaneActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float AspectRatio = 1.666667f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder",
	meta = (UIMin = 0.001f, UIMax = 1.0f))
	float DistanceRate = 0.8f;
};