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
	将当前画面生成到一个UTexture2D.
	注意: 不会自动调用CaptureScene. 需管理其生命周期.
	*/
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	UTexture2D *DrawATexture2D();

	// 将当前画面输出到一个UTexture2D.
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void DrawOnTexture2D(UTexture2D *Texutre2D);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	int TargetWidth = 1920;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	int TargetHeight = 1080;
};
