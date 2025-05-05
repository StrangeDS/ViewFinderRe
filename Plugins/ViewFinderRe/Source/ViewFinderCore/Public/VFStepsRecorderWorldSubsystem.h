#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VFStepsRecorderWorldSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVFStepRecorderDelegate, float, Time);

class IVFStepsRecordInterface;

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API UVFStepsRecorderWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual TStatId GetStatId() const override;

	virtual void OnWorldBeginPlay(UWorld &InWorld) override;
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	FORCEINLINE float GetTime() { return Time; }

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	FORCEINLINE float GetDeltaTime() { return TickInterval; }

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void SubmitStep(UObject *Sender, FVFStepInfo Info);

	UPROPERTY(VisibleAnywhere, Category = "ViewFinder")
	TArray<FVFStepInfo> Infos;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void RecordTransform(USceneComponent *Component);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void UnrecordTransform(USceneComponent *Component);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<AVFTransfromRecorderActor> TransformRecorder;

public:
	void TickForward(float DeltaTime);

	void TickBackward(float DeltaTime);

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFStepRecorderDelegate OnTickTime;

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void RegisterTickable(TScriptInterface<IVFStepsRecordInterface> Target);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void RegisterTransformRecordere(AVFTransfromRecorderActor *Recorder);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void UnregisterTickable(TScriptInterface<IVFStepsRecordInterface> Target);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TArray<TScriptInterface<IVFStepsRecordInterface>> TickTargets;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TArray<TScriptInterface<IVFStepsRecordInterface>> TargetsNeedToAdd;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TArray<TScriptInterface<IVFStepsRecordInterface>> TargetsNeedToRemove;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void StartRewinding();

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFStepRecorderDelegate OnStartRewinding;

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void EndRewinding();

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFStepRecorderDelegate OnEndRewinding;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	bool bIsRewinding = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float RewindFactor = 3.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float RewindCurFactor = RewindFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float Time = TIME_MIN;

	// 自定义tick间隔, 默认为20帧/秒
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float TickInterval = 0.05f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float TimeSinceLastTick = 0.f;

	static inline const float TIME_MAX = 1e6;				// 计时器时间的最大值(最大时间)
	static inline const float TIME_MIN = 1e-6;				// 计时器时间的最小值(开始时间)
	static inline const int SecondsOfAnHour = 60 * 60 * 20; // 一小时帧数, 默认20帧(TickInterval)
#if WITH_EDITOR
	static inline const int SizeRecommended = SecondsOfAnHour / 6;
#else
	// 推荐数组大小. 更好的做法是写一个内存分配器: 直接给定10分, 1小时, 1天等阶梯式的内存大小.
	static inline const int SizeRecommended = SecondsOfAnHour;
#endif

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void RewindToLastKey();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float TimeOfRewindToLastKey = 3.0f;

	FTimerHandle RewindHandle;
};
