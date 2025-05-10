#include "VFPhotoCatcherPref.h"

#include "VFCommon.h"
#include "VFPhotoCaptureComponent.h"

AVFPhotoCatcherPref::AVFPhotoCatcherPref() : Super()
{
}

void AVFPhotoCatcherPref::OnConstruction(const FTransform &Transform)
{
    Super::OnConstruction(Transform);

    PhotoCapture->ShowOnlyActors = OnlyActorsCatched;
    if (OnlyActorsCatched.Num() > 0)
    {
        PhotoCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
    }
    else
    {
        PhotoCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
    }
}

TArray<UPrimitiveComponent *> AVFPhotoCatcherPref::GetOverlapComps_Implementation()
{
    auto Comps = Super::GetOverlapComps_Implementation();
    if (OnlyActorsCatched.Num() > 0)
    {
        for (auto It = Comps.CreateIterator(); It; ++It)
        {
            auto Actor = (*It)->GetOwner();
            if (!OnlyActorsCatched.Contains(Actor))
                It.RemoveCurrent();
        }
    }

    return Comps;
}

#if WITH_EDITOR

void AVFPhotoCatcherPref::RecollectShowOnlyActors()
{
    const FScopedTransaction Transaction(FText::FromString(TEXT("RecollectShowOnlyActors")));

    OnlyActorsCatched.Reset();
    auto Comps = Super::GetOverlapComps_Implementation();
    for (auto &Comp : Comps)
    {
        OnlyActorsCatched.AddUnique(Comp->GetOwner());
    }
    PhotoCapture->ShowOnlyActors = OnlyActorsCatched;
}

#include "LevelInstance/LevelInstanceSubsystem.h"
#include "LevelInstance/LevelInstanceInterface.h"

#include "VFPhoto2D.h"
#include "VFPhoto3D.h"
#include "VFDynamicMeshComponent.h"

void AVFPhotoCatcherPref::SaveAPhotoInEditor()
{
    TArray<AActor *> ActorsToMove;
    TArray<UPrimitiveComponent *> CompsOverlapped = Super::GetOverlapComps();

    auto RestoreSourceComponents = [this, &CompsOverlapped]()
    {
        // 移除用于替换的VFDMComps
        TArray<UVFDynamicMeshComponent *> VFDMComps;
        for (auto &Comp : CompsOverlapped)
        {
            auto Original = Comp->GetOwner();
            VFDMComps.Reset();
            Original->GetComponents<UVFDynamicMeshComponent>(VFDMComps);
            for (auto &VFDMComp : VFDMComps)
            {
                if (!CompsOverlapped.Contains(VFDMComp))
                {
                    VFDMComp->RestoreSourceComponent();
                    VFDMComp->DestroyComponent();
                }
            }
        }
    };

    auto AfterFailure = [this, &ActorsToMove, &RestoreSourceComponents](const FString &Reason)
    {
        // 移除复制出来的Actor
        for (auto Actor : ActorsToMove)
            GetWorld()->EditorDestroyActor(Actor, false);

        RestoreSourceComponents();
        GEditor->RedrawLevelEditingViewports();
        VF_LOG(Error, TEXT("%s Fails: %s."), __FUNCTIONW__, *Reason);
    };

    ULevelInstanceSubsystem *LevelInstanceSubsystem = GetWorld()->GetSubsystem<ULevelInstanceSubsystem>();
    if (!LevelInstanceSubsystem)
    {
        AfterFailure(TEXT("invalid LevelInstanceSubsystem"));
        return;
    }

    bool bUseTempRenderTarget = PhotoCapture->TextureTarget == nullptr;
    if (bUseTempRenderTarget)
        PhotoCapture->Init();

    auto Photo2DRoot = TakeAPhoto();
    auto Photo3DRoot = Photo2DRoot->GetPhoto3D();
    Photo3DRoot->GetAttachedActors(ActorsToMove, true, true);
    // 最前/最后?
    ActorsToMove.AddUnique(Photo2DRoot);
    ActorsToMove.AddUnique(Photo3DRoot);

    RestoreSourceComponents();
    if (bUseTempRenderTarget)
    {
        PhotoCapture->TextureTarget = nullptr;
    }

    // 在移动到新的关卡实例后, Photo2D对Photo3D的引用关系会消失, 需要修复.
    // 使用Tags来修复引用关系, 配对的Photo2D和Photo3D给一组guid.
    TArray<FName> Guids;
    for (auto &Actor : ActorsToMove)
    {
        if (auto Photo2D = Cast<AVFPhoto2D>(Actor))
        {
            if (Photo2D->bIsRecursive)
                continue;
            if (!Photo2D->GetPhoto3D())
                continue;

            FName Guid = FName(*FGuid::NewGuid().ToString());
            Guids.Add(Guid);
            Photo2D->Tags.Add(Guid);
            Photo2D->GetPhoto3D()->Tags.Add(Guid);
        }
    }

    FNewLevelInstanceParams Params;
    Params.Type = ELevelInstanceCreationType::LevelInstance;
    Params.PivotType = ELevelInstancePivotType::Actor;
    Params.PivotActor = Photo2DRoot;
    Params.SetForceExternalActors(LevelInstanceSubsystem->GetWorld()->IsPartitionedWorld());
    ILevelInstanceInterface *LevelInstance = LevelInstanceSubsystem->CreateLevelInstanceFrom(ActorsToMove, Params);
    if (!LevelInstance)
    {
        AfterFailure(TEXT("fails to create LevelInstance."));
        return;
    }

    // 修复Photo2D对Photo3D的引用关系
    LevelInstance->EnterEdit();
    ULevel *Level = LevelInstance->GetLoadedLevel();
    {
        TMap<FName, AVFPhoto2D *> Photo2DMap;
        TMap<FName, AVFPhoto3D *> Photo3DMap;
        for (auto &Actor : Level->Actors)
        {
            if (auto Photo2D = Cast<AVFPhoto2D>(Actor))
            {
                for (auto &Guid : Guids)
                {
                    if (Photo2D->Tags.Contains(Guid))
                    {
                        Photo2DMap.Add(Guid, Photo2D);
                        Photo2D->Tags.Remove(Guid);
                        break;
                    }
                }
            }
            else if (auto Photo3D = Cast<AVFPhoto3D>(Actor))
            {
                for (auto &Guid : Guids)
                {
                    if (Photo3D->Tags.Contains(Guid))
                    {
                        Photo3DMap.Add(Guid, Photo3D);
                        Photo3D->Tags.Remove(Guid);
                        break;
                    }
                }
            }
        }

        for (auto &[Tag, Photo2D] : Photo2DMap)
        {
            auto Photo3D = Photo3DMap[Tag];
            Photo2D->SetPhoto3D(Photo3D);
            Photo3D->MarkPackageDirty();
            Photo2D->MarkPackageDirty();
        }
    }

    for (auto &Actor : Level->Actors)
    {
        auto Photo2D = Cast<AVFPhoto2D>(Actor);
        if (!Photo2D)
            continue;
    }

    LevelInstance->ExitEdit();
    VF_LOG(Log, TEXT("SaveAPhotoInEditor successed."));
}

#endif