#include "VFTransfromRecorderActor.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Components/PrimitiveComponent.h"

#include "VFCommon.h"
#include "VFFunctions.h"
#include "VFTransformRecordVolume.h"

bool FVFTransCompInfo::operator==(const FVFTransCompInfo &Other) const
{
	if (Component != Other.Component)
		return false;
	if (Velocity != Other.Velocity)
		return false;
	return Transform.Equals(Other.Transform);
}

AVFTransfromRecorderActor::AVFTransfromRecorderActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
}

void AVFTransfromRecorderActor::BeginPlay()
{
	Super::BeginPlay();

	if (!GetStepsRecorder())
	{
		VF_LOG(Error, TEXT("%s invalid StepsRecorder."));
		return;
	}

	StepsRecorder->RegisterTransformRecorder(this);

	Steps.Reserve(UVFStepsRecorderWorldSubsystem::GetSizeRecommended());
	ReCollectComponents();
}

void AVFTransfromRecorderActor::AddToRecord(USceneComponent *Component)
{
	check(Component);
	Components.AddUnique(Component);
}

void AVFTransfromRecorderActor::RemoveFromRecord(USceneComponent *Component)
{
	Components.RemoveSwap(Component);
}

void AVFTransfromRecorderActor::ReCollectComponents_Implementation()
{
	Components.Reset();
	CompInfoMap.Reset();

	if (!GetStepsRecorder())
	{
		VF_LOG(Error, TEXT("%s invalid StepsRecorder."));
		return;
	}

	TArray<AActor *> Volumes;
	UGameplayStatics::GetAllActorsOfClass(
		this,
		TSubclassOf<AActor>(AVFTransformRecordVolume::StaticClass()),
		Volumes);
	for (const auto &Volume : Volumes)
	{
		auto RecordVolume = Cast<AVFTransformRecordVolume>(Volume);
		Components.Append(RecordVolume->GetComponents());
	}

	TArray<FVFTransCompInfo> Infos;
	CompInfoMap.Reserve(Components.Num());
	Infos.Reserve(Components.Num());
	for (const TObjectPtr<USceneComponent> &Comp : Components)
	{
		FVFTransCompInfo Info(Comp);
		CompInfoMap.Add(Comp, Info);
		Infos.Add(Info);
	}
	FVFTransStepInfo StepInfo(UVFStepsRecorderWorldSubsystem::TIME_MIN, Infos);
	Steps.Add(StepInfo);
}

void AVFTransfromRecorderActor::TickForward_Implementation(float Time)
{
	TArray<FVFTransCompInfo> Infos;
	for (auto It = Components.CreateIterator(); It; It++)
	{
		auto Comp = *It;
		if (!IsValid(Comp))
		{
			VF_LOG(Warning, TEXT("%s: Comp销毁"), __FUNCTIONW__);
			It.RemoveCurrent();
			continue;
		}

		auto Info = FVFTransCompInfo(Comp);
		if (!CompInfoMap.Contains(Comp) || CompInfoMap[Comp] != Info)
		{
			CompInfoMap.Add(Comp, Info);
			Infos.Add(Info);
		}
	}

	if (!Infos.IsEmpty())
	{
		FVFTransStepInfo StepInfo(StepsRecorder->GetTime(), Infos);
		Steps.Add(StepInfo);
	}
}

void AVFTransfromRecorderActor::TickBackward_Implementation(float Time)
{
	while (Steps.Num() > 1)
	{
		FVFTransStepInfo &StepInfo = Steps.Last();
		if (StepInfo.Time < Time)
			break;

		for (const auto &Info : StepInfo.Infos)
		{
			auto Comp = Info.Component;
			if (!CompInfoMap.Contains(Comp))
			{
				VF_LOG(Warning, TEXT("%s: Comp未找到"), __FUNCTIONW__);
				continue;
			}
			CompInfoMap[Comp] = Info;
		}

		Steps.Pop(false);
	}

	auto TimeLast = Steps.Last().Time;

	for (auto It = CompInfoMap.CreateIterator(); It; ++It)
	{
		auto &[Comp, Info] = *It;
		if (!IsValid(Comp))
		{
			VF_LOG(Warning, TEXT("%s: Comp销毁"), __FUNCTIONW__);
			It.RemoveCurrent();
			continue;
		}

		auto Delta = StepsRecorder->GetDeltaTime() / (StepsRecorder->GetTime() - TimeLast);
		Delta = FMath::Min(Delta, 1.0f);
		Comp->SetWorldTransform(UVFFunctions::TransformLerp(Comp->GetComponentTransform(), Info.Transform, Delta));
		Comp->ComponentVelocity = FMath::Lerp(Comp->GetComponentVelocity(), Info.Velocity, Delta);
	}
}