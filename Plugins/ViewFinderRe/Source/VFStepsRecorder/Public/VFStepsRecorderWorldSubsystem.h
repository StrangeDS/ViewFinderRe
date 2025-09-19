#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "VFStepsRecorderWorldSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVFStepsRecorderDelegate, float, Time);

UENUM(BlueprintType)
enum class EVFStepsRecorderSubsystemCheckMode : uint8
{
	RequireRewinding, // 必须处于回放状态
	IgnoreRewinding,  // 不检查回放状态
};

class IVFStepsRecordInterface;
class AVFTransfromRecorderActor;

UCLASS(ClassGroup = (ViewFinder))
class VFSTEPSRECORDER_API UVFStepsRecorderWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual TStatId GetStatId() const override;

	virtual void OnWorldBeginPlay(UWorld &InWorld) override;

	virtual void Tick(float DeltaTime) override;

	virtual void Deinitialize() override;

public:
	UFUNCTION(BlueprintPure, Category = "ViewFinder")
	FORCEINLINE float GetTime() { return Time; }

	UFUNCTION(BlueprintPure, Category = "ViewFinder")
	FORCEINLINE float GetDeltaTime() { return TickInterval; }

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void SubmitStep(UObject *Sender, FVFStepInfo Info);

	UPROPERTY(VisibleAnywhere, Category = "ViewFinder")
	TArray<FVFStepInfo> Infos;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void RecordTransform(USceneComponent *Component,
						 const FString &Channel = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void UnrecordTransform(USceneComponent *Component,
						   const FString &Channel = TEXT(""));

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TMap<FString, TObjectPtr<AVFTransfromRecorderActor>> TransformRecorderMap;

public:
	void TickForward(float DeltaTime);

	void TickBackward(float DeltaTime);

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFStepsRecorderDelegate OnTickTime;

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void RegisterTickable(const TScriptInterface<IVFStepsRecordInterface> &Target);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void UnregisterTickable(const TScriptInterface<IVFStepsRecordInterface> &Target);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	bool IsTickableRegistered(const TScriptInterface<IVFStepsRecordInterface> &Target);

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
	FVFStepsRecorderDelegate OnStartRewinding;

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void EndRewinding();

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFStepsRecorderDelegate OnEndRewinding;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	bool bIsRewinding = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float RewindCurFactor = 3.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float Time = TIME_MIN;

	// 自定义tick间隔, 默认为20帧/秒
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float TickInterval = 0.05f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float TimeSinceLastTick = 0.f;

public:
	// <0将使用当前时间
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void SetTimeOfStart(float Start = -1.0f);

	// <0将使用TIME_MAX
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void SetTimeOfEnd(float End = -1.0f);

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFStepsRecorderDelegate OnSetTimeOfStart;

	// 计时器回溯的最小值(开始时间)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float TimeOfStart = TIME_MIN;

	// 计时器回溯记录的最大值(结束时间)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float TimeOfEnd = TIME_MAX;

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static float GetTimeOfMin();

	// <0将使用TIME_MAX
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static float GetTimeOfMax();

	// 计时器时间的最小值(建议开始时间)
	static inline const float TIME_MIN = 1e-6;

	// 计时器时间的最大值(建议最大时间)
	static inline const float TIME_MAX = 1e6;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static int GetSizeRecommended();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void RewindToLastKey();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float TimeOfRewindToLastKey = 3.0f;

	FTimerHandle RewindHandle;

	// 实时获取, 适合低频率, 事件驱动
public:
	UFUNCTION(BlueprintPure, Category = "ViewFinder",
			  meta = (WorldContext = "WorldContext"))
	static UVFStepsRecorderWorldSubsystem *GetStepsRecorder(
		const UObject *WorldContext,
		const EVFStepsRecorderSubsystemCheckMode &Mode = EVFStepsRecorderSubsystemCheckMode::RequireRewinding);
};

// 使用成员变量的方式, 需要头文件
// 成员变量缓存, 适合高频率访问
// VarName为成员变量名
#define DECLARE_STEPSRECORDER_SUBSYSTEM_ACCESSOR(VarName)                         \
private:                                                                          \
	mutable TObjectPtr<UVFStepsRecorderWorldSubsystem> VarName = nullptr;         \
                                                                                  \
public:                                                                           \
	/* 主访问器：返回指针并自动验证条件 */                                        \
	FORCEINLINE UVFStepsRecorderWorldSubsystem *GetStepsRecorder() const          \
	{                                                                             \
		if (!VarName && IsAtRuntime())                                            \
		{                                                                         \
			VarName = GetWorld()->GetSubsystem<UVFStepsRecorderWorldSubsystem>(); \
		}                                                                         \
		return VarName;                                                           \
	}                                                                             \
                                                                                  \
private:                                                                          \
	/* 初始化条件封装 */                                                          \
	FORCEINLINE bool IsAtRuntime() const                                          \
	{                                                                             \
		const UWorld *World = GetWorld();                                         \
		return World && (World->WorldType == EWorldType::Game ||                  \
						 World->WorldType == EWorldType::PIE);                    \
	}
