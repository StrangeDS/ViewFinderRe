// Copyright 2026, StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFPostProcessComponent.generated.h"

class UMaterialInterface;

UENUM(BlueprintType)
enum class EVFStencilRule : uint8
{
	None = 0,
	Original,
	Fixed,
	Growing,
	MAX
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VFPHOTOCATCHER_API UVFPostProcessComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVFPostProcessComponent();

#if WITH_EDITOR
	virtual void BeginPlay() override;
#endif

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	void ClearSceneCapturePostProcess();

public:
	UFUNCTION(BlueprintPure, Category = "ViewFinder")
	bool IsAnyRule();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void AddOrUpdateSceneCapturePostProcess(USceneCaptureComponent2D *SceneCapture);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void RemoveSceneCapturePostProcess(USceneCaptureComponent2D *SceneCapture);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void AddOrUpdateCameraPostProcess(UCameraComponent *Camera);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void RemoveCameraPostProcess(UCameraComponent *Camera);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void SetStencilValueNext(UPrimitiveComponent *Comp);
	virtual void SetStencilValueNext_Implementation(UPrimitiveComponent *Comp);

	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "ViewFinder")
	int GetStencilValueNext(int Stencil);
	virtual int GetStencilValueNext_Implementation(int Stencil);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	int StencilBase = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	EVFStencilRule Rule = EVFStencilRule::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UMaterialInterface> PostProcess;
};