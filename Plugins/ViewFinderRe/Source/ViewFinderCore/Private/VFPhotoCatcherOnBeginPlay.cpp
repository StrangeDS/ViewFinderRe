#include "VFPhotoCatcherOnBeginPlay.h"

#include "VFPhoto2D.h"
#include "VFPhotoCaptureComponent.h"

AVFPhotoCatcherOnBeginPlay::AVFPhotoCatcherOnBeginPlay() : Super()
{
    PhotoCapture->bCaptureEveryFrame = false;
    PhotoCapture->bCaptureOnMovement = false;
}

void AVFPhotoCatcherOnBeginPlay::BeginPlay()
{
    Super::BeginPlay();

    if (bOnlyCatchChildActors)
    {
        TArray<AActor *> Actors;
        GetAttachedActors(Actors, true, true);
        PhotoCapture->ShowOnlyActors = Actors;
        PhotoCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
    }

    // UE不支持BeginPlay定义顺序. 若直接在此进行, 碰撞检测到的网格极可能没有BeginPlay,
    // 且UVFStepsRecorderWorldSubsystem的TransformRecorder也没有进行注册
    GetWorldTimerManager().SetTimerForNextTick([this](){
        auto Photo2D = TakeAPhoto();
        Photo2D->AddActorLocalTransform(PhotoSpawnPoint);
        Destroy();
    });
}