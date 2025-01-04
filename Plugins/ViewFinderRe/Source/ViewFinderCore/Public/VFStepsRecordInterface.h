#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VFStepsRecordInterface.generated.h"

USTRUCT(BlueprintType)
struct FVFStepInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	FString Info;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bIsKeyFrame = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	float Time = 0.f;

	// 由UVFStepsRecorderWorldSubsystem填入
	UPROPERTY(BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<UObject> Sender;
};

UINTERFACE(MinimalAPI, Blueprintable)
class UVFStepsRecordInterface : public UInterface
{
	GENERATED_BODY()
};

class VIEWFINDERCORE_API IVFStepsRecordInterface
{
	GENERATED_BODY()

public:
	// 访问者模式
	// 由UVFStepsRecorderWorldSubsystem进行tick(forward/backword), 需要手动注册
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void TickForward(float Time);
	virtual void TickForward_Implementation(float Time) {};

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void TickBackward(float Time);
	virtual void TickBackward_Implementation(float Time) {};

	// 向UVFStepsRecorderWorldSubsystem::SubmitChanges()的, 在此退回,
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	bool StepBack(FVFStepInfo &StepInfo);
	virtual bool StepBack_Implementation(FVFStepInfo &StepInfo) { return false; };

	template <typename T>
	static FString EnumToString(T &&Step);

	template <typename T>
	static T StringToEnum(FString &String);
};

template <typename T>
inline FString IVFStepsRecordInterface::EnumToString(T &&Step)
{
    return FString::FromInt((int)Step);
}

template <typename T>
inline T IVFStepsRecordInterface::StringToEnum(FString &String)
{
    return (T)FCString::Atoi(*String);
}
