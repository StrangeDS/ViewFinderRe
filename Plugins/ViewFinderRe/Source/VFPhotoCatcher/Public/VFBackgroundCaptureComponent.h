// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VFPhotoCaptureComponent.h"
#include "VFBackgroundCaptureComponent.generated.h"

class AVFPlaneActor;

// Dedicated for background
UCLASS(Blueprintable, ClassGroup = (ViewFinder), meta = (BlueprintSpawnableComponent))
class VFPHOTOCATCHER_API UVFBackgroundCaptureComponent : public UVFPhotoCaptureComponent
{
	GENERATED_BODY()

public:
	UVFBackgroundCaptureComponent();

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext &Context) const override;
#endif

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