#include "VFTransfromRecorderActor.h"

#include "Components/PrimitiveComponent.h"

#include "VFLog.h"

static FTransform TransformLerp(const FTransform &Original, const FTransform &Target, float delta)
{
	FRotator Rot = FMath::Lerp(Original.Rotator(), Target.Rotator(), delta);
	FVector Loc = FMath::Lerp(Original.GetLocation(), Target.GetLocation(), delta);
	return FTransform(Rot, Loc, Original.GetScale3D());
}

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
	if (!GetStepsRecorder())
	{
		VF_LOG(Error, TEXT("%s invalid StepsRecorder."));
		return;
	}

    // 没有手动调用Clear, 而是直接使用赋值.
    auto Comps = K2_GetComponentsByClass(CompClassToCollect);
    Components.Empty(Comps.Num());
    for (auto &Comp: Comps)
    {
        check(Cast<USceneComponent>(Comp));
        Components.Emplace(Cast<USceneComponent>(Comp));
    }

	TArray<FVFTransCompInfo> Infos;
	Infos.Reserve(Components.Num());
	CompInfoMap.Empty(Components.Num());
	for (const TObjectPtr<USceneComponent> &Comp : Components)
	{
		FVFTransCompInfo Info(Comp);
		CompInfoMap.Add(Comp, Info);
		Infos.Add(Info);
	}
	FVFTransStepInfo StepInfo(UVFStepsRecorderWorldSubsystem::TIME_MIN, Infos);
	Steps.Add(StepInfo);
}

void AVFTransfromRecorderActor::ClearComponents()
{
	Components.Reset();
}

void AVFTransfromRecorderActor::ClearInfoMap()
{
    CompInfoMap.Reset();
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
		Comp->SetWorldTransform(TransformLerp(Comp->GetComponentTransform(), Info.Transform, Delta));
		Comp->ComponentVelocity = FMath::Lerp(Comp->GetComponentVelocity(), Info.Velocity, Delta);
	}
}