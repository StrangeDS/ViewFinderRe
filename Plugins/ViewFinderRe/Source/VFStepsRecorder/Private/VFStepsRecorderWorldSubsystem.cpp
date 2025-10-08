#include "VFStepsRecorderWorldSubsystem.h"

#include "TimerManager.h"

#include "VFLog.h"
#include "VFStepsRecordInterface.h"
#include "VFStepsRecorderDeveloperSettings.h"
#include "VFTransfromRecorderActor.h"

TStatId UVFStepsRecorderWorldSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UVFStepsRecorderWorldSubsystem, STATGROUP_Tickables);
}

void UVFStepsRecorderWorldSubsystem::OnWorldBeginPlay(UWorld &InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    auto Settings = GetDefault<UVFStepsRecorderDeveloperSettings>();
    TickInterval = Settings->StepsRecorderTickInterval;
    RewindCurFactor = Settings->StepsRecorderRewindFactor;

    Infos.Reserve(10 * GetSizeRecommended());
}

void UVFStepsRecorderWorldSubsystem::Tick(float DeltaTime)
{
    if (TargetsNeedToAdd.Num() > 0)
    {
        TickTargets.Append(TargetsNeedToAdd);
        TargetsNeedToAdd.Reset();
    }
    else if (TargetsNeedToRemove.Num() > 0)
    {
        TickTargets.RemoveAll(
            [this](
                const TScriptInterface<IVFStepsRecordInterface> &Target)
            { return TargetsNeedToRemove.Contains(Target); });
        TargetsNeedToRemove.Reset();
    }

    TimeSinceLastTick += bIsRewinding ? DeltaTime * RewindCurFactor : DeltaTime;
    while (TimeSinceLastTick > TickInterval)
    {
        Time += bIsRewinding ? -TickInterval : TickInterval;
        if (bIsRewinding)
        {
            TickBackward(TickInterval);
        }
        else
        {
            TickForward(TickInterval);
        }
        TimeSinceLastTick -= TickInterval;
        OnTickTime.Broadcast(Time);
    }
}

void UVFStepsRecorderWorldSubsystem::Deinitialize()
{
    for (auto &[Channel, TransformRecorder] : TransformRecorderMap)
    {
        TransformRecorder->Destroy();
    }
    TransformRecorderMap.Empty();
}

void UVFStepsRecorderWorldSubsystem::SubmitStep(UObject *Sender, FVFStepInfo Info)
{
    Info.Sender = Sender;
    if (Info.Time < 0.f)
        Info.Time = Time;
    Infos.Add(Info);
}

void UVFStepsRecorderWorldSubsystem::TickForward(float DeltaTime)
{
    Time = FMath::Min(Time, TimeOfEnd);

    for (auto It = TickTargets.CreateIterator(); It; ++It)
    {
        auto Target = *It;
        if (!IsValid(Target.GetObject()))
        {
            It.RemoveCurrent();
        }
        else
        {
            IVFStepsRecordInterface::Execute_TickForward(Target.GetObject(), Time);
        }
    }
}

void UVFStepsRecorderWorldSubsystem::TickBackward(float DeltaTime)
{
    if (Time <= TimeOfStart)
    {
        Time = TimeOfStart;
        EndRewinding();
        return;
    }

    while (!Infos.IsEmpty())
    {
        auto Info = Infos.Last();
        if (Info.Time < Time)
            break;

        if (!IsValid(Info.Sender))
            continue;

        bool handled = IVFStepsRecordInterface::Execute_StepBack(Info.Sender, Info);
        if (!handled)
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("UVFStepsRecorderWorldSubsystem::TickBackward(): StepInfo(%s) from %s does not get handled"),
                *Info.Info,
                *Info.Sender->GetName());
        }
        Infos.Pop(false);
    }

    for (auto &Target : TickTargets)
    {
        if (IsValid(Target.GetObject()))
            IVFStepsRecordInterface::Execute_TickBackward(Target.GetObject(), Time);
    }
}

void UVFStepsRecorderWorldSubsystem::RecordTransform(
    USceneComponent *Component, const FString &Channel)
{
    check(Component && Component->Mobility == EComponentMobility::Movable);

    if (!TransformRecorderMap.Contains(Channel))
    {
        auto TransformRecorder = GetWorld()->SpawnActor<AVFTransfromRecorderActor>();
        TransformRecorderMap.Emplace(Channel, TransformRecorder);
    }
    TransformRecorderMap[Channel]->AddToRecord(Component);
}

void UVFStepsRecorderWorldSubsystem::UnrecordTransform(
    USceneComponent *Component, const FString &Channel)
{
    check(Component && Component->Mobility == EComponentMobility::Movable);

    if (!TransformRecorderMap.Contains(Channel))
    {
        VF_LOG(Error, TEXT("%s invalid channel."), __FUNCTIONW__);
        return;
    }

    if (!TransformRecorderMap[Channel]->IsBegingRecorded(Component))
    {
        VF_LOG(Error, TEXT("%s Comp is not being Recorded."), __FUNCTIONW__);
        return;
    }
    TransformRecorderMap[Channel]->RemoveFromRecord(Component);
}

void UVFStepsRecorderWorldSubsystem::RegisterTickable(const TScriptInterface<IVFStepsRecordInterface> &Target)
{
    if (IsValid(Target.GetObject()))
        TargetsNeedToAdd.AddUnique(Target);
}

void UVFStepsRecorderWorldSubsystem::UnregisterTickable(const TScriptInterface<IVFStepsRecordInterface> &Target)
{
    if (ensure(TickTargets.Contains(Target)))
    {
        TargetsNeedToRemove.AddUnique(Target);
    }
}

bool UVFStepsRecorderWorldSubsystem::IsTickableRegistered(const TScriptInterface<IVFStepsRecordInterface> &Target)
{
    return TickTargets.Contains(Target);
}

void UVFStepsRecorderWorldSubsystem::StartRewinding()
{
    if (bIsRewinding)
        return;

    TimeSinceLastTick = 0.f;
    OnStartRewinding.Broadcast(Time);
    bIsRewinding = true;
}

void UVFStepsRecorderWorldSubsystem::EndRewinding()
{
    if (!bIsRewinding)
        return;

    TimeSinceLastTick = 0.f;
    OnEndRewinding.Broadcast(Time);
    bIsRewinding = false;
}

void UVFStepsRecorderWorldSubsystem::SetTimeOfStart(float Start)
{
    if (Start < 0.f)
        Start = Time;

    if (Start > Time)
        VF_LOG(Warning, TEXT("%s: TimeOfStart(%f) is later than current(%f)"),
               __FUNCTIONW__, Start, Time);

    TimeOfStart = Start;
}

void UVFStepsRecorderWorldSubsystem::SetTimeOfEnd(float End)
{
    if (End < 0.f)
        End = TIME_MAX;

    TimeOfEnd = End;
}

float UVFStepsRecorderWorldSubsystem::GetTimeOfMin()
{
    return TIME_MIN;
}

float UVFStepsRecorderWorldSubsystem::GetTimeOfMax()
{
    return TIME_MAX;
}

int UVFStepsRecorderWorldSubsystem::GetSizeRecommended()
{
    return GetDefault<UVFStepsRecorderDeveloperSettings>()->StepsRecorderSizeRecommended;
}

void UVFStepsRecorderWorldSubsystem::RewindToLastKey()
{
    float TimeOfTarget = TimeOfStart;
    for (int i = Infos.Num() - 1; i >= 0; i--)
    {
        if (Infos[i].bIsKeyFrame)
        {
            TimeOfTarget = Infos[i].Time;
            break;
        }
    }

    TimeOfTarget = TimeOfTarget - TickInterval;
    float TimeSpan = Time - TimeOfTarget;
    auto Settings = GetDefault<UVFStepsRecorderDeveloperSettings>();
    float TimeOfBackard = TimeSpan / Settings->StepsRecorderRewindFactor;
    TimeOfBackard = FMath::Min(TimeOfBackard, Settings->StepsRecorderTimeOfRewindToLastKey);
    RewindCurFactor = TimeSpan / TimeOfBackard;

    GetWorld()->GetTimerManager().SetTimer(
        RewindHandle, [this, TimeSpan]()
        {
            RewindCurFactor = GetDefault<UVFStepsRecorderDeveloperSettings>()->StepsRecorderRewindFactor;
            EndRewinding(); },
        TimeOfBackard,
        false);
    StartRewinding();
}

UVFStepsRecorderWorldSubsystem *UVFStepsRecorderWorldSubsystem::GetStepsRecorder(
    const UObject *WorldContext,
    const EVFStepsRecorderSubsystemCheckMode &Mode)
{
    const UWorld *World = WorldContext->GetWorld();
    bool IsRuntime = World && (World->WorldType == EWorldType::Game ||
                               World->WorldType == EWorldType::PIE);
    if (!IsRuntime)
    {
        VF_LOG(Warning, TEXT("%s should only be called in runtime."), __FUNCTIONW__);
        return nullptr;
    }

    auto System = World->GetSubsystem<UVFStepsRecorderWorldSubsystem>();
    switch (Mode)
    {
    case EVFStepsRecorderSubsystemCheckMode::RequireRewinding:
        return System->bIsRewinding ? nullptr : System;
    case EVFStepsRecorderSubsystemCheckMode::IgnoreRewinding:
    default:
        return System;
    }
}
