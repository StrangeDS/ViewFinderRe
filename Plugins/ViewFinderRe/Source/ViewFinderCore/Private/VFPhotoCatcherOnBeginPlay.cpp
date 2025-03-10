#include "VFPhotoCatcherOnBeginPlay.h"

#include "VFPhoto2D.h"
#include "VFPhotoCaptureComponent.h"

AVFPhotoCatcherOnBeginPlay::AVFPhotoCatcherOnBeginPlay() : Super()
{
    PhotoCapture->bCaptureEveryFrame = false;
    PhotoCapture->bCaptureOnMovement = false;
    PhotoCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
}

void AVFPhotoCatcherOnBeginPlay::OnConstruction(const FTransform &Transform)
{
    Super::OnConstruction(Transform);

#if WITH_EDITOR
    CompsInEditor.Reset();
    if (bOnlyCollectInSameLevel)
        CollectCompsInSameLevel();
    else
        CollectCompsInLevels();
#endif
}

void AVFPhotoCatcherOnBeginPlay::BeginPlay()
{
    Super::BeginPlay();

    GetWorldTimerManager().SetTimerForNextTick([this]()
                                               {
                                                   auto Photo2D = TakeAPhoto();
                                                   Photo2D->AddActorLocalTransform(PhotoSpawnPoint);
                                                   SetActorHiddenInGame(bHideChildActors);
                                                   HideCurLevel(); });
}

TArray<UPrimitiveComponent *> AVFPhotoCatcherOnBeginPlay::GetOverlapComps_Implementation()
{
    if (CompsInEditor.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("AVFPhotoCatcherOnBeginPlay(%s): 没有预处理碰撞组件"));
    }
    return CompsInEditor;
}

void AVFPhotoCatcherOnBeginPlay::HideCurLevel()
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
void AVFPhotoCatcherOnBeginPlay::CollectCompsInLevels()
{
    auto Comps = Super::GetOverlapComps_Implementation();
    for (auto &Comp : Comps)
    {
        CompsInEditor.AddUnique(Comp);
    }
    UpdateOnlyActorsCatched();
}

void AVFPhotoCatcherOnBeginPlay::CollectCompsInSameLevel()
{
    ULevel *Level = GetLevel();
    auto Comps = Super::GetOverlapComps_Implementation();
    for (auto &Comp : Comps)
    {
        if (Comp->GetOwner()->GetLevel() == Level)
            CompsInEditor.AddUnique(Comp);
    }
    UpdateOnlyActorsCatched();
}

void AVFPhotoCatcherOnBeginPlay::ClearCompsInEditor()
{
    CompsInEditor.Reset();
}

void AVFPhotoCatcherOnBeginPlay::UpdateOnlyActorsCatched()
{
    OnlyActorsCatched.Reset();
    for (auto &Comp : CompsInEditor)
    {
        OnlyActorsCatched.AddUnique(Comp->GetOwner());
    }
    PhotoCapture->ShowOnlyActors = OnlyActorsCatched;
}
#endif
