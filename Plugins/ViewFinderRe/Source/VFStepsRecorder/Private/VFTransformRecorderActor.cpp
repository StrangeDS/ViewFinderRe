// Copyright StrangeDS. All Rights Reserved.

#include "VFTransformRecorderActor.h"

#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetSystemLibrary.h"

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
	if (Visible != Other.Visible)
		return false;
	if (Velocity != Other.Velocity)
		return false;
	return Transform.Equals(Other.Transform);
}

bool FVFTransCompInfo::IsChanged(const FVFTransCompInfo &InfoNew) const
{
	check(Component == InfoNew.Component);

	if (Visible != InfoNew.Visible)
		return true;

	if (!Visible)
		return false;

	return Velocity != InfoNew.Velocity || !Transform.Equals(InfoNew.Transform);
}

AVFTransformRecorderActor::AVFTransformRecorderActor()
{
	PrimaryActorTick.bCanEverTick = false;
	CompClassToCollect = UPrimitiveComponent::StaticClass();

	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
}

void AVFTransformRecorderActor::BeginPlay()
{
	Super::BeginPlay();

	if (!GetStepsRecorder())
	{
		VF_LOG(Error, TEXT("%s invalid StepsRecorder."), __FUNCTIONW__);
		return;
	}

	GetStepsRecorder()->RegisterTickable(this);
	Steps.Reserve(UVFStepsRecorderWorldSubsystem::GetSizeRecommended());
	ReCollectComponents();
}

void AVFTransformRecorderActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearComponents();
	ClearInfoMap();

	Super::EndPlay(EndPlayReason);
}

void AVFTransformRecorderActor::AddToRecord(USceneComponent *Component)
{
	check(Component);
	Components.AddUnique(Component);
}

void AVFTransformRecorderActor::RemoveFromRecord(USceneComponent *Component)
{
	Components.RemoveSwap(Component);
	CompInfoMap.Remove(Component);
}

bool AVFTransformRecorderActor::IsBegingRecorded(USceneComponent *Component)
{
	return Components.Contains(Component);
}

void AVFTransformRecorderActor::ReCollectComponents_Implementation()
{
	if (!IsValid(CompClassToCollect))
	{
		VF_LOG(Error, TEXT("%s invalid CompClassToCollect."), __FUNCTIONW__);
		return;
	}

	SetActorEnableCollision(true);

	Components.Reset();
	TArray<AActor *> ActorsToIgnore = {GetOwner()};
	TArray<UPrimitiveComponent *> ChildComps;
	GetComponents<UPrimitiveComponent>(ChildComps);
	for (auto &ChildComp : ChildComps)
	{
		TArray<UPrimitiveComponent *> Overlaps;
		bool Result = UKismetSystemLibrary::ComponentOverlapComponents(
			Cast<UPrimitiveComponent>(ChildComp),
			ChildComp->GetComponentTransform(),
			ObjectTypes,
			CompClassToCollect,
			ActorsToIgnore,
			Overlaps);

		for (auto &Overlap : Overlaps)
		{
			if (Overlap->Implements<UVFStepsRecordInterface>())
				continue;
			else if (Overlap->Mobility != EComponentMobility::Movable)
				continue;
			else
				Components.Emplace(Overlap);
		}
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
	FVFTransStepInfo StepInfo(StepsRecorder->GetTime(), Infos);
	Steps.Add(StepInfo);

	SetActorEnableCollision(false);
}

void AVFTransformRecorderActor::ClearComponents()
{
	Components.Reset();
}

void AVFTransformRecorderActor::ClearInfoMap()
{
	CompInfoMap.Reset();
}

void AVFTransformRecorderActor::TickForward_Implementation(float Time)
{
	TArray<FVFTransCompInfo> Infos;
	for (auto It = Components.CreateIterator(); It; It++)
	{
		auto Comp = *It;
		if (!IsValid(Comp))
		{
			VF_LOG(Warning, TEXT("%s: Comp is invalid."), __FUNCTIONW__);
			It.RemoveCurrent();
			continue;
		}

		auto Info = FVFTransCompInfo(Comp);
		if (!CompInfoMap.Contains(Comp) || CompInfoMap[Comp].IsChanged(Info))
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

void AVFTransformRecorderActor::TickBackward_Implementation(float Time)
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
				VF_LOG(Warning, TEXT("%s: fail to find Comp."), __FUNCTIONW__);
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
			VF_LOG(Warning, TEXT("%s: Comp is invalid."), __FUNCTIONW__);
			It.RemoveCurrent();
			continue;
		}

		auto Delta = StepsRecorder->GetDeltaTime() / (StepsRecorder->GetTime() - TimeLast);
		Delta = FMath::Min(Delta, 1.0f);
		Comp->SetWorldTransform(TransformLerp(Comp->GetComponentTransform(), Info.Transform, Delta));
		Comp->ComponentVelocity = FMath::Lerp(Comp->GetComponentVelocity(), Info.Velocity, Delta);
	}
}