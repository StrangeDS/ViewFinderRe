#include "VFPhotoCatcherPref.h"

#include "Materials/MaterialInstanceConstant.h"

#include "VFCommon.h"
#include "VFPhotoCaptureComponent.h"

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
    if (IterationTimes < 0 || IterationTimes > 10)
    {
        VF_LOG(Error, TEXT("%s invalid IterationTimes(%i)."),
               __FUNCTIONW__, IterationTimes);
        return;
    }

    ULevelInstanceSubsystem *LevelInstanceSubsystem =
        GetWorld()->GetSubsystem<ULevelInstanceSubsystem>();
    if (!LevelInstanceSubsystem)
    {
        VF_LOG(Error, TEXT("%s invalid LevelInstanceSubsystem."), __FUNCTIONW__);
        return;
    }

    // 临时数据
    TArray<UPrimitiveComponent *> CompsOverlapped = GetOverlapComps();
    TArray<AActor *> ActorsToMove;
    UTexture2D *Texture2D = nullptr;
    UMaterialInstanceConstant *MIConstant = nullptr;
    bool bUseTempRenderTarget = PhotoCapture->TextureTarget == nullptr;

    // 清理临时数据
    auto ClearTemporary = [this,
                           &CompsOverlapped,
                           Texture2D,
                           MIConstant,
                           &bUseTempRenderTarget]()
    {
        // 移除原场景用于替换的VFDMComps
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
        CompsOverlapped.Reset();

        if (Texture2D)
        {
            Texture2D->ClearFlags(RF_Public | RF_Standalone);
            Texture2D->MarkAsGarbage();
        }

        if (MIConstant)
        {
            MIConstant->ClearFlags(RF_Public | RF_Standalone);
            MIConstant->MarkAsGarbage();
        }

        if (bUseTempRenderTarget)
            PhotoCapture->TextureTarget = nullptr;
    };

    // 失败资源回收
    auto AfterFailure = [this,
                         &ActorsToMove,
                         &ClearTemporary](const FString &Reason)
    {
        // 移除复制出来的Actor
        for (auto Actor : ActorsToMove)
        {
            GetWorld()->EditorDestroyActor(Actor, false);
        }
        ActorsToMove.Reset();
        GEditor->RedrawLevelEditingViewports();

        VF_LOG(Error, TEXT("%s Fails: %s."), __FUNCTIONW__, *Reason);

        ClearTemporary();
    };

    // 支持编辑器中指定渲染目标
    if (bUseTempRenderTarget)
        PhotoCapture->Init();

    // 拍照生成
    auto Photo2DRoot = TakeAPhoto();
    auto Photo3DRoot = Photo2DRoot->GetPhoto3D();

    // 纹理和常量材质实例生成
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

    // Photo2D的收集和处理
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
            Photo2D->StaticMesh->SetMaterial(MaterialIndex, MIConstant);

            if (Photo2D->bIsRecursive)
            {
                Photo2D->SetActorRelativeScale3D(RelativeScale3D);
                auto Comps = Photo2D->StaticMesh->GetAttachChildren();
                for (auto &Comp : Comps)
                {
                    if (auto VFDMComp = Cast<UVFDynamicMeshComponent>(Comp))
                    {
                        VFDMComp->SourceComponent = Photo2D->StaticMesh;
                        VFDMComp->SetMaterial(MaterialIndex, MIConstant);
                        break;
                    }
                }
            }
        }

        // // 迭代照相
        // for (int i = 0; i < IterationTimes; ++i)
        // {
        //     PhotoCapture->CaptureScene();
        //     PhotoCapture->TextureTarget->UpdateTexture2D(Texture2D, Texture2D->Source.GetFormat());
        //     Texture2D->MarkPackageDirty();
        // }

        // 迭代照相
        // for (int i = 0; i < IterationTimes; ++i)
        // {
        //     PhotoCapture->CaptureScene();
        //     Texture2D = PhotoCapture->DrawATexture2D();
        //     MIConstant->SetTextureParameterValueEditorOnly(TextureName, Texture2D);
        // }
        // Texture2D->ClearFlags(RF_Transient);
        // Texture2D->SetFlags(RF_Public | RF_Standalone);
    }

    // 迁移前准备
    ActorsToMove.AddUnique(Photo2DRoot);
    ActorsToMove.AddUnique(Photo3DRoot);
    Photo3DRoot->GetAttachedActors(ActorsToMove, false, true);

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

    // 迁移到关卡实例
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
    // 编辑关卡实例
    LevelInstance->EnterEdit();

    // 需要的数据
    AVFPhoto2D *Photo2DRootInLevel = nullptr;
    TArray<AVFPhoto2D *> Photo2DsInSame;

    ULevel *Level = LevelInstance->GetLoadedLevel();
    {
        // 修复Photo2D对Photo3D的引用关系
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
                if (Photo2D->MaterialInstance && !Photo2D->bIsRecursive)
                {
                    Photo2DRootInLevel = Photo2D;
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

    if (!Photo2DRootInLevel)
    {
        AfterFailure(TEXT("fails to find Photo2DRootInLevel."));
        return;
    }

    UTexture2D *TextureAsset = nullptr;
    UMaterialInstanceConstant *MIContantAsset = nullptr;

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
        if (!TextureAsset)
        {
            AfterFailure(TEXT("Save textureAsset fails."));
            return;
        }

        AssetName = FString::Printf(TEXT("%s_MIC"), *AssetPrefix);
        MIContantAsset = Cast<UMaterialInstanceConstant>(AssetTools.Get().DuplicateAsset(
            AssetName,
            AssetPath,
            MIConstant));
        if (!MIContantAsset)
        {
            AfterFailure(TEXT("save MIContantAsset fails."));
            return;
        }

        MIContantAsset->SetTextureParameterValueEditorOnly(TextureName, TextureAsset);
    }

    // 替换成常量材质资产
    {
        Photo2DsInSame.Add(Photo2DRootInLevel);
        FVector RelativeScale3D = Photo2DRootInLevel->GetActorRelativeScale3D();
        for (auto &Photo2D : Photo2DsInSame)
        {
            if (Photo2D->bIsRecursive)
            {
                Photo2D->SetActorRelativeScale3D(RelativeScale3D);
                for (auto &Comp : Photo2D->StaticMesh->GetAttachChildren())
                {
                    if (auto VFDMComp = Cast<UVFDynamicMeshComponent>(Comp))
                    {
                        VFDMComp->SourceComponent = Photo2D->StaticMesh;
                        VFDMComp->SetMaterial(MaterialIndex, MIContantAsset);
                        break;
                    }
                }
            }
            Photo2D->StaticMesh->SetMaterial(MaterialIndex, MIContantAsset);
            Photo2D->MaterialInstance = nullptr;
            Photo2D->MarkPackageDirty();
        }
    }

    LevelInstance->ExitEdit();

    // 原场景迭代拍照
    {
        TArray<AVFPhoto2D *> Photo2DsOrigial;
        for (auto &Comp : CompsOverlapped)
        {
            auto Actor = Comp->GetOwner();
            if (auto Photo2D = Cast<AVFPhoto2D>(Actor))
            {
                if (Photo2D->bIsRecursive)
                {
                    Photo2DsOrigial.AddUnique(Photo2D);
                }
            }
        }
        for (auto &Photo2D : Photo2DsOrigial)
        {
            Photo2D->StaticMesh->SetMaterial(MaterialIndex, MIContantAsset);
        }
        PhotoCapture->CaptureScene();
        PhotoCapture->TextureTarget->UpdateTexture2D(TextureAsset, TextureAsset->Source.GetFormat());
    }

    ClearTemporary();
    VF_LOG(Log, TEXT("SaveAPhotoInEditor successed."));
}

void AVFPhotoCatcherPref::UpdateMIC()
{
    if (!Texture2DAsset)
    {
        VF_LOG(Error,
               TEXT("%s invalid Texture2DAsset. Select or Prefabricate first."),
               __FUNCTIONW__);
        return;
    }

    if (!MIConstantAsset)
    {
        VF_LOG(Error,
               TEXT("%s invalid MaterialInstanceConstant. Select or Prefabricate first."),
               __FUNCTIONW__);
        return;
    }

    bool bUseTempRenderTarget = PhotoCapture->TextureTarget == nullptr;
    if (bUseTempRenderTarget)
        PhotoCapture->Init();

    for (int i = 0; i < IterationTimes; ++i)
    {
        PhotoCapture->CaptureScene();
        PhotoCapture->TextureTarget->UpdateTexture2D(Texture2DAsset, Texture2DAsset->Source.GetFormat());
    }

    if (bUseTempRenderTarget)
        PhotoCapture->TextureTarget = nullptr;
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