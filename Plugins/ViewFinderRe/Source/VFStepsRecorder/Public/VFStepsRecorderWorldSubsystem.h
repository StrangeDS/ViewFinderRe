// Copyright 2026, StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "VFStepsRecorderWorldSubsystem.generated.h"

class IVFStepsRecordInterface;
class AVFTransformRecorderActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVFStepsRecorderDelegate, float, Time);

UENUM(BlueprintType)
enum class EVFStepsRecorderSubsystemCheckMode : uint8
{
	NotRewinding, // Must be in rewind state, in game world.
	InGameWorld,  // Don't check rewind state, in game world.
};

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

	// Stable insert for maintains order
	UFUNCTION(BlueprintCallable, Category = "ViewFinder",
			  meta = (DefaultToSelf = "Sender"))
	void SubmitStep(UObject *Sender, FVFStepInfo Info);

	// Ordered array
	UPROPERTY(VisibleAnywhere, Category = "ViewFinder")
	TArray<FVFStepInfo> Infos;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void RecordTransform(USceneComponent *Component,
						 const FString &Channel = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void UnrecordTransform(USceneComponent *Component,
						   const FString &Channel = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	bool IsTransformRecorded(USceneComponent *Component,
							 const FString &Channel = TEXT(""));

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TMap<FString, TObjectPtr<AVFTransformRecorderActor>> TransformRecorderMap;

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

	// Custom tick interval, default is 20 frames per second
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float TickInterval = 0.05f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float TimeSinceLastTick = 0.f;

	// TIME_MIN < TimeOfStart <= Time <= TimeOfEnd < TIME_MAX
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float Time = TickInterval;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static float GetTimeOfMin();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static float GetTimeOfMax();

	// Timer minimum time value (recommended start time)
	static inline const float TIME_MIN = 1e-6;

	// Timer maximum time value (recommended max time)
	static inline const float TIME_MAX = 1e6;

public:
	// Use current time when it < TIME_MIN.
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void SetTimeOfStart(float Start = -1.0f);

	// Use TIME_MAX - TickInterval when it < 0.
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void SetTimeOfEnd(float End = -1.0f);

	UFUNCTION(BlueprintPure, Category = "ViewFinder")
	float GetTimeOfStart();

	UFUNCTION(BlueprintPure, Category = "ViewFinder")
	float GetTimeOfEnd();

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFStepsRecorderDelegate OnSetTimeOfStart;

	// Timer rewind minimum value (start time)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float TimeOfStart = Time;

	// Timer rewind maximum value (end time)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float TimeOfEnd = TIME_MAX;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	static int GetSizeRecommended();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void RewindToLastKey();

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void CheckRewoundToLastKeyPoint(float TimeCur);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float TimeOfRewindToLastKey = 3.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float TimeOfRewindingTarget = TimeOfStart;

public:
	/*
	Static function to get.
	By default, can only get when not in rewind state.
	*/
	UFUNCTION(BlueprintPure, Category = "ViewFinder",
			  meta = (WorldContext = "WorldContext"))
	static UVFStepsRecorderWorldSubsystem *GetStepsRecorder(
		const UObject *WorldContext,
		const EVFStepsRecorderSubsystemCheckMode Mode =
			EVFStepsRecorderSubsystemCheckMode::NotRewinding);
};

/*
Using member variable approach, requires header file.
Member variable cache, suitable for high-frequency access.
VarName is the member variable name.
*/
#define DECLARE_STEPSRECORDER_SUBSYSTEM_ACCESSOR(VarName)                         \
private:                                                                          \
	mutable TObjectPtr<UVFStepsRecorderWorldSubsystem> VarName = nullptr;         \
                                                                                  \
public:                                                                           \
	/* Main accessor: return pointer and automatically verify conditions */       \
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
	/* Initialization condition wrapper */                                        \
	FORCEINLINE bool IsAtRuntime() const                                          \
	{                                                                             \
		const UWorld *World = GetWorld();                                         \
		return World && (World->WorldType == EWorldType::Game ||                  \
						 World->WorldType == EWorldType::PIE);                    \
	}