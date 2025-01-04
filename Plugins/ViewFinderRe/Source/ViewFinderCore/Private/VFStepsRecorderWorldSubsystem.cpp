#include "VFStepsRecorderWorldSubsystem.h"

#include "Kismet/GameplayStatics.h"

#include "VFTransfromRecorderActor.h"

TStatId UVFStepsRecorderWorldSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UVFStepsRecorderWorldSubsystem, STATGROUP_Tickables);
}

void UVFStepsRecorderWorldSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
    Super::Initialize(Collection);
}

void UVFStepsRecorderWorldSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UVFStepsRecorderWorldSubsystem::OnWorldBeginPlay(UWorld &InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
}

void UVFStepsRecorderWorldSubsystem::Tick(float DeltaTime)
{
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
        if (!Target)
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
        if (Target)
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
    if (ensure(TransformRecorder))
    {
        if (Component->Mobility == EComponentMobility::Movable)
            TransformRecorder->AddToRecord(Component);
    }
}

void UVFStepsRecorderWorldSubsystem::UnrecordTransform(USceneComponent *Component)
{
    if (ensure(TransformRecorder))
    {
        TransformRecorder->RemoveFromRecord(Component);
    }
}

void UVFStepsRecorderWorldSubsystem::RegisterTickable(TScriptInterface<IVFStepsRecordInterface> Target)
{
    check(Target.GetObject()->Implements<UVFStepsRecordInterface>());
    TickTargets.AddUnique(Target);
}

void UVFStepsRecorderWorldSubsystem::RegisterTransformRecordere(AVFTransfromRecorderActor *Recorder)
{
    check(Recorder);
    TransformRecorder = Recorder;
    RegisterTickable(TransformRecorder);
}

void UVFStepsRecorderWorldSubsystem::UnregisterTickable(TScriptInterface<IVFStepsRecordInterface> Target)
{
    TickTargets.RemoveSwap(Target);
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

const float UVFStepsRecorderWorldSubsystem::TIME_MAX = 1.0e6;

const float UVFStepsRecorderWorldSubsystem::TIME_MIN = 0.f;