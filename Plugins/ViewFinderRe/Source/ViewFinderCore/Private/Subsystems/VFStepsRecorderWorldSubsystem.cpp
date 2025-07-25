#include "VFStepsRecorderWorldSubsystem.h"

#include "TimerManager.h"

#include "VFCommon.h"
#include "VFStepsRecordInterface.h"
#include "VFTransfromRecorderActor.h"
#include "ViewFinderReSettings.h"

TStatId UVFStepsRecorderWorldSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UVFStepsRecorderWorldSubsystem, STATGROUP_Tickables);
}

void UVFStepsRecorderWorldSubsystem::OnWorldBeginPlay(UWorld &InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    auto Setting = GetDefault<UViewFinderReSettings>();
    TickInterval = Setting->StepsRecorderTickInterval;
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

    RewindCurFactor = GetDefault<UViewFinderReSettings>()->StepsRecorderRewindFactor;
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
    Info.Sender = Sender;
    if (Info.Time < 0.f)
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

int UVFStepsRecorderWorldSubsystem::GetSizeRecommended()
{
    return GetDefault<UViewFinderReSettings>()->StepsRecorderSizeRecommended;
}

void UVFStepsRecorderWorldSubsystem::RewindToLastKey()
{
    for (int i = Infos.Num() - 1; i >= 0; i--)
    {
        if (Infos[i].bIsKeyFrame)
        {
            float TimeSpan = (Time - Infos[i].Time + TickInterval);
            TimeSpan = FMath::Max(TimeSpan, TickInterval);
            float Speed = TimeSpan / GetDefault<UViewFinderReSettings>()
                                         ->StepsRecorderTimeOfRewindToLastKey;
            Speed = FMath::Max(Speed, 1.0f);
            RewindCurFactor = Speed;
            StartRewinding();
            GetWorld()->GetTimerManager().SetTimer(
                RewindHandle, [this, TimeSpan]()
                {
                    RewindCurFactor = GetDefault<UViewFinderReSettings>()
                    ->StepsRecorderRewindFactor;
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
