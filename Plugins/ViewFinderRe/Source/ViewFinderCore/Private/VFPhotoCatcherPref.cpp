#include "VFPhotoCatcherPref.h"

#include "Materials/MaterialInstanceConstant.h"

#include "VFCommon.h"
#include "VFPhotoCaptureComponent.h"

AVFPhotoCatcherPref::AVFPhotoCatcherPref() : Super()
{
#if WITH_EDITORONLY_DATA
    static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> MaterialInstanceSelector(
        TEXT("/ViewFinderRe/Materials/Photo/MI_PhotoWithBorder.MI_PhotoWithBorder"));
    MaterialInstanceSource = MaterialInstanceSelector.Object;
#endif
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
#include "AssetToolsModule.h"

#include "VFPhoto2D.h"
#include "VFPhoto3D.h"
#include "VFDynamicMeshComponent.h"

UMaterialInstanceConstant *SaveDynamicAsConstant(FAssetToolsModule &AssetTools,
                                                 UMaterialInstanceDynamic *MIDynamic,
                                                 const FString &AssetPath,
                                                 const FString &AssetName)
{
    if (!MIDynamic)
    {
        VF_LOG(Error, TEXT("%s invalid MIDynamic."), __FUNCTIONW__);
        return nullptr;
    }

    auto ParentMaterial = MIDynamic->Parent;
    if (!IsValid(ParentMaterial))
    {
        VF_LOG(Error, TEXT("%s invalid ParentMaterial"), __FUNCTIONW__);
        return nullptr;
    }

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

    UMaterialInstanceConstant *SavedMIC = Cast<UMaterialInstanceConstant>(
        AssetTools.Get().DuplicateAsset(AssetName, AssetPath, MIConstant));

    return SavedMIC;
}

void AVFPhotoCatcherPref::SaveAPhotoInEditor()
{
    TArray<AActor *> ActorsToMove;
    TArray<UPrimitiveComponent *> CompsOverlapped = Super::GetOverlapComps();
    UMaterialInstanceDynamic *MIDynamic = nullptr;
    UMaterialInstanceConstant *MIContant = nullptr;
    UTexture2D *Texture2D = nullptr;

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

    auto AfterFailure = [this,
                         &ActorsToMove,
                         &RestoreSourceComponents](const FString &Reason)
    {
        // 移除复制出来的Actor
        for (auto Actor : ActorsToMove)
            GetWorld()->EditorDestroyActor(Actor, false);
        GEditor->RedrawLevelEditingViewports();
        VF_LOG(Error, TEXT("%s EditorDestroyActor: %i."), __FUNCTIONW__, ActorsToMove.Num());

        RestoreSourceComponents();
        VF_LOG(Error, TEXT("%s Fails: %s."), __FUNCTIONW__, *Reason);
    };

    if (!MaterialInstanceSource)
    {
        AfterFailure(TEXT("invalid MaterialInstanceSource"));
        return;
    }

    if (IterationTimes < 0 || IterationTimes > 10)
    {
        AfterFailure(TEXT("invalid IterationTimes"));
        return;
    }

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

    {
        Texture2D = PhotoCapture->DrawATexture2D();
        if (!Texture2D)
        {
            AfterFailure(TEXT("invalid Texture2D"));
            return;
        }
		Texture2D->ClearFlags(RF_Transient);
		Texture2D->SetFlags(RF_Public | RF_Standalone);
    }
    
    ActorsToMove.AddUnique(Photo2DRoot);
    ActorsToMove.AddUnique(Photo3DRoot);
    Photo3DRoot->GetAttachedActors(ActorsToMove, false, true);

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
    Params.SetExternalActors(LevelInstanceSubsystem->GetWorld()->IsPartitionedWorld());
    ILevelInstanceInterface *LevelInstance = LevelInstanceSubsystem->CreateLevelInstanceFrom(ActorsToMove, Params);
    if (!LevelInstance)
    {
        AfterFailure(TEXT("fails to create LevelInstance."));
        return;
    }

    // 修复Photo2D对Photo3D的引用关系
    LevelInstance->EnterEdit();

    TArray<AVFPhoto2D *> Photo2DsInSame;
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

                // 最外层Photo2D
                if (Photo2D->MaterialInstance)
                {
                    MIDynamic = Photo2D->MaterialInstance;
                    Photo2D->MaterialInstance = nullptr;
                    Photo2DsInSame.Add(Photo2D);
                }

                // 内层递归Photo2D
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

    {
        const TSoftObjectPtr<UWorld> &Asset = LevelInstance->GetWorldAsset();
        FAssetToolsModule &AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

        FString NewAssetPath = FPaths::GetPath(Asset.ToString());
        FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
        FString BaseName = FPaths::GetBaseFilename(LevelInstance->GetWorldAssetPackage());
        FString NewAssetPrefix = FString::Printf(TEXT("%s_%s"), *BaseName, *Timestamp);
        if (!MIDynamic)
        {
            AfterFailure(TEXT("invalid  MIDynamic."));
            return;
        }

        FString NewAssetName = FString::Printf(TEXT("%s_MIC"), *NewAssetPrefix);
        MIContant = SaveDynamicAsConstant(AssetTools, MIDynamic, NewAssetPath, NewAssetName);
        if (!MIContant)
        {
            AfterFailure(TEXT("SaveDynamicAsConstant fails."));
            return;
        }

        NewAssetName = FString::Printf(TEXT("%s_Texture2D"), *NewAssetPrefix);
        UTexture2D *TextureAsset = Cast<UTexture2D>(AssetTools.Get().DuplicateAsset(
            NewAssetName,
            NewAssetPath,
            Texture2D));
        if (!TextureAsset)
        {
            AfterFailure(TEXT("Save textureAsset fails."));
            return;
        }

        MIContant->SetTextureParameterValueEditorOnly(TEXT("Texture"), TextureAsset);
    }

    // 设置常量材质实例
    {
        for (auto &Photo2D : Photo2DsInSame)
        {
            Photo2D->StaticMesh->SetMaterial(0, MIContant);

            if (Photo2D->bIsRecursive)
            {
                auto Comps = Photo2D->StaticMesh->GetAttachChildren();
                for (auto &Comp : Comps)
                {
                    if (auto VFDMComp = Cast<UVFDynamicMeshComponent>(Comp))
                    {
                        VFDMComp->SourceComponent = Photo2D->StaticMesh;
                        // VFDMComp->UpdateMaterials();
                        VFDMComp->SetMaterial(0, MIContant);
                        break;
                    }
                }
            }
            Photo2D->MarkPackageDirty();
        }
    }

    LevelInstance->ExitEdit();
    VF_LOG(Log, TEXT("SaveAPhotoInEditor successed."));
}

AVFPhoto2D *AVFPhotoCatcherPref::TakeAPhoto_Implementation()
{
    if (GEngine && !GEngine->IsEditor())
    {
        VF_LOG(Error, TEXT("%s should only call in editor and not runtime."));
        return nullptr;
    }

    return Super::TakeAPhoto_Implementation();
}

#endif