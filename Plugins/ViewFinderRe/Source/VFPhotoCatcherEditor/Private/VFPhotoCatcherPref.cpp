// Copyright StrangeDS. All Rights Reserved.

#include "VFPhotoCatcherPref.h"

#include "Materials/MaterialInstanceConstant.h"

#include "VFLog.h"
#include "VFPlaneActor.h"
#include "VFPhotoCaptureComponent.h"
#include "VFBackgroundCaptureComponent.h"

AVFPhotoCatcherPref::AVFPhotoCatcherPref() : Super()
{
#if WITH_EDITOR
    bIsEditorOnlyActor = true;
#endif
}

#if WITH_EDITOR
void AVFPhotoCatcherPref::OnConstruction(const FTransform &Transform)
{
    Super::OnConstruction(Transform);

    ActorsToIgnore.AddUnique(this);
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
    if (GetWorld() && GetWorld()->WorldType != EWorldType::Editor)
    {
        VF_LOG(Error, TEXT("%s must be called in editor."), __FUNCTIONW__);
        return {};
    }

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
#include "AssetToolsModule.h"

#include "VFPhoto2D.h"
#include "VFPhoto3D.h"
#include "VFDynamicMeshComponent.h"

UMaterialInstanceConstant *CreateMICFromMID(UMaterialInstanceDynamic *MIDynamic)
{
    check(MIDynamic);
    TObjectPtr<UMaterialInterface> ParentMaterial = MIDynamic->Parent;
    check(ParentMaterial);

    UMaterialInstanceConstant *MIConstant = NewObject<UMaterialInstanceConstant>(
        GetTransientPackage(),
        MakeUniqueObjectName(GetTransientPackage(),
                             UMaterialInstanceConstant::StaticClass()));
    MIConstant->SetParentEditorOnly(ParentMaterial);

    TArray<FMaterialParameterInfo> OutParameterInfo;
    TArray<FGuid> OutParameterIds;
    MIDynamic->GetAllScalarParameterInfo(OutParameterInfo, OutParameterIds);
    for (const FMaterialParameterInfo &Info : OutParameterInfo)
    {
        float Value;
        if (MIDynamic->GetScalarParameterValue(Info.Name, Value))
        {
            MIConstant->SetScalarParameterValueEditorOnly(Info.Name, Value);
        }
    }
    MIDynamic->GetAllVectorParameterInfo(OutParameterInfo, OutParameterIds);
    for (const FMaterialParameterInfo &Info : OutParameterInfo)
    {
        FLinearColor Value;
        if (MIDynamic->GetVectorParameterValue(Info.Name, Value))
        {
            MIConstant->SetVectorParameterValueEditorOnly(Info.Name, Value);
        }
    }
    MIDynamic->GetAllTextureParameterInfo(OutParameterInfo, OutParameterIds);
    for (const FMaterialParameterInfo &Info : OutParameterInfo)
    {
        UTexture *Value = nullptr;
        if (MIDynamic->GetTextureParameterValue(Info.Name, Value))
        {
            MIConstant->SetTextureParameterValueEditorOnly(Info.Name, Value);
        }
    }

    return MIConstant;
}

#include "Engine/TextureRenderTarget2D.h"

void AVFPhotoCatcherPref::PrefabricateAPhotoLevel()
{
    ULevelInstanceSubsystem *LevelInstanceSubsystem =
        GetWorld()->GetSubsystem<ULevelInstanceSubsystem>();
    if (!LevelInstanceSubsystem)
    {
        VF_LOG(Error, TEXT("%s invalid LevelInstanceSubsystem."), __FUNCTIONW__);
        return;
    }

    bool bPhotoCaptureTempRT = PhotoCapture->TextureTarget == nullptr;
    bool bBgPhotoCaptureTempRT = BackgroundCapture->TextureTarget == nullptr;
    if (bPhotoCaptureTempRT)
    {
        PhotoCapture->Init();
    }
    if (bBgPhotoCaptureTempRT)
    {
        BackgroundCapture->Init();
    }

    // Temporary Data
    TArray<UPrimitiveComponent *> CompsOverlapped = GetOverlapComps();
    TArray<AActor *> ActorsToMove;
    UTexture2D *Texture2D = nullptr;
    UTexture2D *BgTexture2D = nullptr;
    UMaterialInstanceConstant *MIConstant = nullptr;
    UMaterialInstanceConstant *BgMIConstant = nullptr;

    // Cleanup temporary data
    auto ClearTemporary = [this,
                           &CompsOverlapped,
                           Texture2D,
                           BgTexture2D,
                           MIConstant,
                           BgMIConstant,
                           &bPhotoCaptureTempRT,
                           &bBgPhotoCaptureTempRT]()
    {
        // Remove VFDMComps used for replacement in the original ULevel
        TArray<UVFDynamicMeshComponent *> VFDMComps;
        for (auto &Comp : CompsOverlapped)
        {
            // Overlap-detected dynamic meshes indicate pre-existence.
            if (Cast<UVFDynamicMeshComponent>(Comp))
                continue;

            // delete newly created ones and restore their SourceComponent.
            Comp->GetOwner()->GetComponents<UVFDynamicMeshComponent>(VFDMComps, false);
            for (auto &VFDMComp : VFDMComps)
            {
                if (CompsOverlapped.Contains(VFDMComp->GetSourceComponent()))
                {
                    VFDMComp->RestoreSourceComponent();
                    VFDMComp->DestroyComponent();
                }
            }
        }
        VFDMComps.Reset();
        CompsOverlapped.Reset();

        if (Texture2D)
        {
            Texture2D->ClearFlags(RF_Public | RF_Standalone);
            Texture2D->MarkAsGarbage();
        }
        if (BgTexture2D)
        {
            BgTexture2D->ClearFlags(RF_Public | RF_Standalone);
            BgTexture2D->MarkAsGarbage();
        }

        if (MIConstant)
        {
            MIConstant->ClearFlags(RF_Public | RF_Standalone);
            MIConstant->MarkAsGarbage();
        }
        if (BgMIConstant)
        {
            BgMIConstant->ClearFlags(RF_Public | RF_Standalone);
            BgMIConstant->MarkAsGarbage();
        }

        if (bPhotoCaptureTempRT)
        {
            PhotoCapture->TextureTarget = nullptr;
        }
        if (bBgPhotoCaptureTempRT)
        {
            BackgroundCapture->TextureTarget = nullptr;
        }
    };

    // Resource recycling upon failure
    auto AfterFailure = [this,
                         &ActorsToMove,
                         &ClearTemporary](const FString &Reason)
    {
        // Remove duplicated Actors
        for (auto Actor : ActorsToMove)
        {
            GetWorld()->EditorDestroyActor(Actor, false);
        }
        ActorsToMove.Reset();
        GEditor->RedrawLevelEditingViewports();

        VF_LOG(Error, TEXT("%s Fails: %s."), __FUNCTIONW__, *Reason);

        ClearTemporary();
    };

    // Generate Photo.
    auto Photo2DRoot = TakeAPhoto();
    auto Photo3DRoot = Photo2DRoot->GetPhoto3D();

    // Generate texture and constant material instance.
    {
        PhotoCapture->CaptureScene();
        Texture2D = PhotoCapture->DrawATexture2D();
        if (!Texture2D)
        {
            AfterFailure(TEXT("invalid Texture2D"));
            return;
        }
        Texture2D->ClearFlags(RF_Transient);
        Texture2D->SetFlags(RF_Public | RF_Standalone);
    }
    {
        MIConstant = CreateMICFromMID(Photo2DRoot->MaterialInstance);
        if (!MIConstant)
        {
            AfterFailure(TEXT("invalid MIConstant"));
            return;
        }
        MIConstant->ClearFlags(RF_Transient);
        MIConstant->SetFlags(RF_Public | RF_Standalone);
        MIConstant->SetTextureParameterValueEditorOnly(TextureName, Texture2D);
    }
    {
        BackgroundCapture->CaptureScene();
        BgTexture2D = BackgroundCapture->DrawATexture2D();
        if (!BgTexture2D)
        {
            AfterFailure(TEXT("invalid BgTexture2D"));
            return;
        }
        BgTexture2D->ClearFlags(RF_Transient);
        BgTexture2D->SetFlags(RF_Public | RF_Standalone);
    }
    {
        // Find PlaneActor (typically only one exists, without VFDMComp)
        AVFPlaneActor *PlaneActor = nullptr;
        TArray<AActor *> Actors;
        Photo3DRoot->GetAttachedActors(Actors, true, false);
        for (auto &Actor : Actors)
        {
            if (auto Plane = Cast<AVFPlaneActor>(Actor))
            {
                PlaneActor = Plane;
                break;
            }
        }

        BgMIConstant = CreateMICFromMID(PlaneActor->GetMaterialInstanceInEditor());
        if (!BgMIConstant)
        {
            AfterFailure(TEXT("invalid BgMIConstant"));
            return;
        }
        BgMIConstant->ClearFlags(RF_Transient);
        BgMIConstant->SetFlags(RF_Public | RF_Standalone);
        BgMIConstant->SetTextureParameterValueEditorOnly(TextureName, BgTexture2D);
    }

    // Photo2D collection and processing
    {
        TArray<AActor *> Actors;
        TArray<AVFPhoto2D *> Photo2DsInSame;
        Photo3DRoot->GetAttachedActors(Actors, true, true);
        for (auto &Actor : Actors)
        {
            if (auto Photo2D = Cast<AVFPhoto2D>(Actor))
            {
                if (Photo2D->bIsRecursive)
                    Photo2DsInSame.Add(Photo2D);
            }
        }

        FVector RelativeScale3D = Photo2DRoot->GetActorRelativeScale3D();
        for (auto &Photo2D : Photo2DsInSame)
        {
            Photo2D->GetStaticMeshInEditor()->SetMaterial(MaterialIndex, MIConstant);

            if (Photo2D->bIsRecursive)
            {
                Photo2D->SetActorRelativeScale3D(RelativeScale3D);
                auto Comps = Photo2D->GetStaticMeshInEditor()->GetAttachChildren();
                for (auto &Comp : Comps)
                {
                    if (auto VFDMComp = Cast<UVFDynamicMeshComponent>(Comp))
                    {
                        VFDMComp->SourceComponent = Photo2D->GetStaticMeshInEditor();
                        VFDMComp->SetMaterial(MaterialIndex, MIConstant);
                        break;
                    }
                }
            }
        }
    }

    // Pre-migration preparation
    ActorsToMove.AddUnique(Photo2DRoot);
    ActorsToMove.AddUnique(Photo3DRoot);
    Photo3DRoot->GetAttachedActors(ActorsToMove, false, true);

    /*
    After moving to a new level instance,
    reference relationships between Photo2D and Photo3D are lost and need repair.
    Use Tags to restore references, assigning a shared GUID to paired Photo2D and Photo3D.
    */
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

    // Migrate to a new ULevel
    FNewLevelInstanceParams Params;
    Params.Type = ELevelInstanceCreationType::LevelInstance;
    Params.PivotType = ELevelInstancePivotType::Actor;
    Params.PivotActor = Photo2DRoot;
    Params.SetExternalActors(LevelInstanceSubsystem->GetWorld()->IsPartitionedWorld());
    ILevelInstanceInterface *LevelInstance = LevelInstanceSubsystem->CreateLevelInstanceFrom(ActorsToMove, Params);
    if (!LevelInstance)
    {
        AfterFailure(TEXT("fails to create LevelInstance."));
        return;
    }

    ActorsToMove.Reset();
    // Edit the level instance
    LevelInstance->EnterEdit();

    // Required preparation
    AVFPhoto2D *Photo2DRootInLevel = nullptr;
    TArray<AVFPhoto2D *> Photo2DsInSame;

    ULevel *Level = LevelInstance->GetLoadedLevel();
    {
        // Restore reference relationships from Photo2D to Photo3D
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

                // Outermost Photo2D
                if (Photo2D->MaterialInstance && !Photo2D->bIsRecursive)
                {
                    Photo2DRootInLevel = Photo2D;
                }

                // Inner recursive Photo2D
                if (Photo2D->bIsRecursive)
                {
                    Photo2DsInSame.Add(Photo2D);
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

    if (!Photo2DRootInLevel)
    {
        AfterFailure(TEXT("fails to find Photo2DRootInLevel."));
        return;
    }

    // Assetization
    UTexture2D *TextureAsset = nullptr;
    UTexture2D *BgTextureAsset = nullptr;
    UMaterialInstanceConstant *MICAsset = nullptr;
    UMaterialInstanceConstant *BgMICAsset = nullptr;
    {
        const TSoftObjectPtr<UWorld> &LevelAsset = LevelInstance->GetWorldAsset();
        FAssetToolsModule &AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

        FString AssetPath = FPaths::GetPath(LevelAsset.ToString());
        FString TimeStamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
        FString BaseName = FPaths::GetBaseFilename(LevelInstance->GetWorldAssetPackage());
        FString AssetPrefix = FString::Printf(TEXT("%s_%s"), *BaseName, *TimeStamp);

        FString AssetName = FString::Printf(TEXT("%s_Texture2D"), *AssetPrefix);
        TextureAsset = Cast<UTexture2D>(AssetTools.Get().DuplicateAsset(
            AssetName,
            AssetPath,
            Texture2D));
        AssetName = FString::Printf(TEXT("%s_BgTexture2D"), *AssetPrefix);
        BgTextureAsset = Cast<UTexture2D>(AssetTools.Get().DuplicateAsset(
            AssetName,
            AssetPath,
            BgTexture2D));
        if (!TextureAsset)
        {
            AfterFailure(TEXT("Save textureAsset fails."));
            return;
        }
        if (!BgTextureAsset)
        {
            AfterFailure(TEXT("Save textureAsset fails."));
            return;
        }

        AssetName = FString::Printf(TEXT("%s_MIC"), *AssetPrefix);
        MICAsset = Cast<UMaterialInstanceConstant>(AssetTools.Get().DuplicateAsset(
            AssetName,
            AssetPath,
            MIConstant));
        AssetName = FString::Printf(TEXT("%s_BgMIC"), *AssetPrefix);
        BgMICAsset = Cast<UMaterialInstanceConstant>(AssetTools.Get().DuplicateAsset(
            AssetName,
            AssetPath,
            BgMIConstant));
        if (!MICAsset)
        {
            AfterFailure(TEXT("save MICAsset fails."));
            return;
        }
        if (!BgMICAsset)
        {
            AfterFailure(TEXT("save MICAsset fails."));
            return;
        }

        MICAsset->SetTextureParameterValueEditorOnly(TextureName, TextureAsset);
        BgMICAsset->SetTextureParameterValueEditorOnly(TextureName, BgTextureAsset);
    }

    // Convert to constant material assets
    {
        // Replace photo2D reference (including recursive photos)
        Photo2DsInSame.Add(Photo2DRootInLevel);
        FVector RelativeScale3D = Photo2DRootInLevel->GetActorRelativeScale3D();
        for (auto &Photo2D : Photo2DsInSame)
        {
            if (Photo2D->bIsRecursive)
            {
                Photo2D->SetActorRelativeScale3D(RelativeScale3D);
                for (auto &Comp : Photo2D->GetStaticMeshInEditor()->GetAttachChildren())
                {
                    if (auto VFDMComp = Cast<UVFDynamicMeshComponent>(Comp))
                    {
                        VFDMComp->SourceComponent = Photo2D->GetStaticMeshInEditor();
                        VFDMComp->SetMaterial(MaterialIndex, MICAsset);
                        break;
                    }
                }
            }
            Photo2D->GetStaticMeshInEditor()->SetMaterial(MaterialIndex, MICAsset);
            Photo2D->MaterialInstance = nullptr;
            Photo2D->MarkPackageDirty();
        }

        // Replace planeActor reference (theoretically only one PlaneActor should appear here)
        {
            auto Photo3D = Photo2DRootInLevel->GetPhoto3D();
            AVFPlaneActor *PlaneActorInLevel = nullptr;
            TArray<AActor *> Actors;
            Photo3D->GetAttachedActors(Actors, true, false);
            for (auto &Actor : Actors)
            {
                if (auto Plane = Cast<AVFPlaneActor>(Actor))
                {
                    PlaneActorInLevel = Plane;
                    break;
                }
            }

            PlaneActorInLevel->Plane->SetMaterial(MaterialIndex, BgMICAsset);
        }
    }

    LevelInstance->ExitEdit();

    ClearTemporary();

    Texture2DAsset = TextureAsset;
    BgTexture2DAsset = BgTextureAsset;
    MIConstantAsset = MICAsset;
    BgMIConstantAsset = BgMICAsset;

    VF_LOG(Log, TEXT("SaveAPhotoInEditor successed."));
}

void AVFPhotoCatcherPref::UpdateMIC()
{
    bool UpdatePhoto = IsValid(Texture2DAsset) && IsValid(MIConstantAsset);
    bool UpdateBackground = IsValid(BgTexture2DAsset) && IsValid(BgMIConstantAsset);
    if (!UpdatePhoto && !UpdateBackground)
    {
        VF_LOG(Error,
               TEXT("%s invalid Assets. Select or Prefabricate first."),
               __FUNCTIONW__);
        return;
    }

    FScopedTransaction Transaction(FText::FromString(TEXT("UpdateMIC")));
    if (UpdatePhoto)
    {
        Texture2DAsset->Modify();
        MIConstantAsset->Modify();

        bool bPhotoCaptureTempRT = PhotoCapture->TextureTarget == nullptr;
        if (bPhotoCaptureTempRT)
        {
            PhotoCapture->Init();
        }

        PhotoCapture->CaptureScene();
        PhotoCapture->TextureTarget->UpdateTexture2D(Texture2DAsset, Texture2DAsset->Source.GetFormat());

        if (bPhotoCaptureTempRT)
        {
            PhotoCapture->TextureTarget = nullptr;
        }

        Texture2DAsset->PostEditChange();
        Texture2DAsset->MarkPackageDirty();
        MIConstantAsset->MarkPackageDirty();
    }

    if (UpdateBackground)
    {
        BgTexture2DAsset->Modify();
        BgMIConstantAsset->Modify();

        bool bPhotoCaptureTempRT = BackgroundCapture->TextureTarget == nullptr;
        if (bPhotoCaptureTempRT)
        {
            BackgroundCapture->Init();
        }

        BackgroundCapture->CaptureScene();
        BackgroundCapture->TextureTarget->UpdateTexture2D(BgTexture2DAsset, BgTexture2DAsset->Source.GetFormat());

        if (bPhotoCaptureTempRT)
        {
            BackgroundCapture->TextureTarget = nullptr;
        }

        BgTexture2DAsset->PostEditChange();
        BgTexture2DAsset->MarkPackageDirty();
        BgMIConstantAsset->MarkPackageDirty();
    }
}

#include "Engine/Engine.h"
AVFPhoto2D *AVFPhotoCatcherPref::TakeAPhoto_Implementation()
{
    if (GEngine && !GEngine->IsEditor())
    {
        VF_LOG(Error, TEXT("%s should only call in editor and not runtime."), __FUNCTIONW__);
        return nullptr;
    }

    return Super::TakeAPhoto_Implementation();
}
#endif