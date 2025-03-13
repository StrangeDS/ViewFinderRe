#include "VFTransfromRecorderActor.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"

#include "VFCommon.h"
#include "VFTransformRecordVolume.h"
#include "VFFunctions.h"

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
}

void AVFTransfromRecorderActor::BeginPlay()
{
	Super::BeginPlay();

	StepRecorder = GetWorld()->GetSubsystem<UVFStepsRecorderWorldSubsystem>();
	check(StepRecorder);
	ReCollectComponents();
	StepRecorder->RegisterTransformRecordere(this);
	
    Steps.Reserve(UVFStepsRecorderWorldSubsystem::SizeRecommended);
}

void AVFTransfromRecorderActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	FVFTransStepInfo StepInfo(StepRecorder->TIME_MIN, Infos);
	Steps.Add(StepInfo);
}

void AVFTransfromRecorderActor::TickForward_Implementation(float Time)
{
	TArray<FVFTransCompInfo> Infos;
	for (auto It = Components.CreateIterator(); It; It++)
	{
		auto Comp = *It;
		if (!Comp)
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
		FVFTransStepInfo StepInfo(StepRecorder->GetTime(), Infos);
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
		if (!Comp) 
		{
			VF_LOG(Warning, TEXT("%s: Comp销毁"), __FUNCTIONW__);
			It.RemoveCurrent();
			continue;
		}

		auto Delta = StepRecorder->GetDeltaTime() / (StepRecorder->GetTime() - TimeLast);
		Delta = FMath::Min(Delta, 1.0f);
		Comp->SetWorldTransform(UVFFunctions::TransformLerp(Comp->GetComponentTransform(), Info.Transform, Delta));
		Comp->ComponentVelocity = FMath::Lerp(Comp->GetComponentVelocity(), Info.Velocity, Delta);
	}
}