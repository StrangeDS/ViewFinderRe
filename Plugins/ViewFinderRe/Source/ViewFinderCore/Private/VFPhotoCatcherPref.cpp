#include "VFPhotoCatcherPref.h"

#include "VFPhoto2D.h"
#include "VFPhotoCaptureComponent.h"

AVFPhotoCatcherPref::AVFPhotoCatcherPref() : Super()
{
    PhotoCapture->bCaptureEveryFrame = false;
    PhotoCapture->bCaptureOnMovement = false;
    PhotoCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
}

void AVFPhotoCatcherPref::BeginPlay()
{
    Super::BeginPlay();

    GetWorldTimerManager().SetTimerForNextTick([this]()
                                               {
                                                   auto Photo2D = TakeAPhoto();
                                                   Photo2D->AddActorLocalTransform(PhotoSpawnPoint);
                                                   SetActorHiddenInGame(true);
                                                   HideCurLevel(); });
}

TArray<UPrimitiveComponent *> AVFPhotoCatcherPref::GetOverlapComps_Implementation()
{
    if (OnlyActorsCatched.IsEmpty())
    {
        UE_LOG(LogTemp, Warning,
               TEXT("%s(%s): 没有预处理碰撞组件."),
               __FUNCTIONW__,
               *GetName());
    }
    auto Comps = Super::GetOverlapComps_Implementation();
    for (auto It = Comps.CreateIterator(); It; ++It)
    {
        auto Actor = (*It)->GetOwner();
        if (!OnlyActorsCatched.Contains(Actor))
            It.RemoveCurrent();
    }
    return Comps;
}

void AVFPhotoCatcherPref::HideCurLevel()
{
    const ULevel *ActorLevel = GetLevel();
    // GetLevel()打印的名字都是一样的, 但它们是不同的对象(指针是不同的)!
    // UE_LOG(LogTemp, Warning, TEXT("ActorLevel: %s, P: %p"), *ActorLevel->GetName(), ActorLevel);

    // 使用ULevelStreaming来进行隐藏
    UWorld *World = GetWorld();
    const TArray<ULevelStreaming *> &StreamingLevels = World->GetStreamingLevels();
    for (ULevelStreaming *StreamingLevel : StreamingLevels)
    {
        if (StreamingLevel && StreamingLevel->GetLoadedLevel() == ActorLevel)
        {
            if (!StreamingLevel->GetShouldBeVisibleFlag())
            {
                UE_LOG(LogTemp, Warning, TEXT("已经被隐藏了."));
            }
            StreamingLevel->SetShouldBeVisible(false);
            break;
        }
    }
}

#if WITH_EDITOR

void AVFPhotoCatcherPref::RecollectActorsWithFrustum()
{
    OnlyActorsCatched.Reset();
    auto Comps = Super::GetOverlapComps_Implementation();
    for (auto &Comp : Comps)
    {
        OnlyActorsCatched.AddUnique(Comp->GetOwner());
    }
    PhotoCapture->ShowOnlyActors = OnlyActorsCatched;
}

#endif