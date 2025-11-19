// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneCaptureComponent2D.h"
#include "VFPhotoCaptureComponent.generated.h"

UCLASS(Blueprintable, ClassGroup = (ViewFinder), meta = (BlueprintSpawnableComponent))
class VFPHOTOCOMMON_API UVFPhotoCaptureComponent : public USceneCaptureComponent2D
{
	GENERATED_BODY()

public:
	UVFPhotoCaptureComponent();

	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void Init();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void ResizeTarget(int Width, int Height);

	UFUNCTION(BlueprintPure, Category = "ViewFinder")
	FORCEINLINE float GetTargetAspectRatio() { return (float)TargetWidth / TargetHeight; };

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void StartDraw();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void EndDraw();

	/*
	Generate the current RT to a UTexture2D.
	Note: CaptureScene is not called automatically. And you must manage its lifecycle.
	*/
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	UTexture2D *DrawATexture2D();

	// Output the current frame to a UTexture2D.
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void DrawOnTexture2D(UTexture2D *Texutre2D);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	int TargetWidth = 1920;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	int TargetHeight = 1080;
};