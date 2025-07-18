#include "VFStepsRecorderWorldSubsystem.h"

#include "TimerManager.h"

#include "VFCommon.h"
#include "VFStepsRecordInterface.h"
#include "VFTransfromRecorderActor.h"

TStatId UVFStepsRecorderWorldSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UVFStepsRecorderWorldSubsystem, STATGROUP_Tickables);
}

void UVFStepsRecorderWorldSubsystem::OnWorldBeginPlay(UWorld &InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    Infos.Reserve(SizeRecommended * 10);
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

void UVFStepsRecorderWorldSubsystem::TickForward(float DeltaTime)
{
    Time = FMath::Min(Time, TIME_MAX);

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
    if (Time <= TIME_MIN)
    {
        Time = TIME_MIN;
        EndRewinding();
        return;
    }

    while (!Infos.IsEmpty())
    {
        auto Info = Infos.Last();
        if (Info.Time < Time)
            break;

        if (!Info.Sender)
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

void UVFStepsRecorderWorldSubsystem::SubmitStep(UObject *Sender, FVFStepInfo Info)
{
    // if (bIsRewinding)   // 是否有必要? 可以在很多地方避免此判断, 但会削减游戏更多的可能性
    //     return;

    Info.Sender = Sender;
    Info.Time = Time;
    Infos.Add(Info);
}

void UVFStepsRecorderWorldSubsystem::RecordTransform(USceneComponent *Component)
{
    if (!IsValid(TransformRecorder))
    {
        VF_LOG(Warning, TEXT("%s: invalid TransformRecorder."), __FUNCTIONW__);
        return;
    }
    if (Component->Mobility == EComponentMobility::Movable)
        TransformRecorder->AddToRecord(Component);
}

void UVFStepsRecorderWorldSubsystem::UnrecordTransform(USceneComponent *Component)
{
    if (!IsValid(TransformRecorder))
    {
        VF_LOG(Warning, TEXT("%s: invalid TransformRecorder."), __FUNCTIONW__);
        return;
    }
    TransformRecorder->RemoveFromRecord(Component);
}

void UVFStepsRecorderWorldSubsystem::RegisterTickable(const TScriptInterface<IVFStepsRecordInterface> &Target)
{
    if (IsValid(Target.GetObject()))
        TargetsNeedToAdd.AddUnique(Target);
}

void UVFStepsRecorderWorldSubsystem::RegisterTransformRecorder(AVFTransfromRecorderActor *Recorder)
{
    check(Recorder);
    TransformRecorder = Recorder;
    RegisterTickable(TransformRecorder);
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

void UVFStepsRecorderWorldSubsystem::RewindToLastKey()
{
    for (int i = Infos.Num() - 1; i >= 0; i--)
    {
        if (Infos[i].bIsKeyFrame)
        {
            float TimeSpan = (Time - Infos[i].Time + TickInterval);
            TimeSpan = FMath::Max(TimeSpan, TickInterval);
            float Speed = TimeSpan / TimeOfRewindToLastKey;
            Speed = FMath::Max(Speed, 1.0f);
            RewindCurFactor = Speed;
            StartRewinding();
            GetWorld()->GetTimerManager().SetTimer(
                RewindHandle, [this, TimeSpan]()
                {
                    RewindCurFactor = RewindFactor;
                    EndRewinding(); },
                FMath::Min(TimeSpan, TimeOfRewindToLastKey),
                false);
            return;
        }
    }
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
