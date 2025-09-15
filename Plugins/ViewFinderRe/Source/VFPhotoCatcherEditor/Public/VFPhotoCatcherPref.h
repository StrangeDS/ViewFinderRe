#pragma once

#include "CoreMinimal.h"
#include "VFPhotoCatcher.h"
#include "VFPhotoCatcherPref.generated.h"

class UMaterialInstanceConstant;

/*
已尝试过的思路:
1. 编辑器中预处理需要拍到的Actors, BeginPlay中延迟一帧走正常拍照流程.
	已解决的问题: 有些是直接在当前Level进行，有些是在单独的Level进行(拍照后需要隐藏当前Level).
	问题: 难以支持递归.
2. 编辑器中将拍照生成的Photo2D, 连带Photo3D创建为一个新的关卡实例(使用ULevelInstanceSubsystem::CreateLevelInstanceFrom()).
	已解决的问题: 移动到新的关卡实例后, Photo2D对Photo3D的引用关系将会断裂, 使用Guid进行重新配对.
	问题: 需要拍递归的照片, 但编辑器下无法使用MID. 使用过两个RenderTarget交互进行拍照(试图绕开读写问题), 但手动流程依然过多, 且容易出错.
3. (使用中)操作与2差不多, 但:
	在重构UVFPhotoCaptureComponent后, 不再依赖渲染目标.
	拍照后将直接生成新的UTexture2D和材质实例, 保存到关卡所在路径.
	由于都是静态纹理, 可以进行递归拍照, 同时直接使用新的材质实例, 将无需BeginPlay中进行各种后续处理.

关于"哪些是递归的照片":
	1. 使用Photo2D子类? 不太好, 又得再实现一次回溯相关逻辑.
	2. 预处理为数组? 不行, 因为在移动到新关卡实例后, 这些引用关系没有任何用处.
	3. Photo3D下的都是? 不行, 存在照片中照片是另外的场景.
	4. 没有连接Photo3D的? 还不错, 但如果未来玩法需要这种无Photo3D的呢?
	5. (使用中的)Photo2D直接新增属性表示? 还不错, 稍微臃肿.

递归Photo2D何时, 如何获得Photo3D:
	1. OnCopyAfterCopiedForPhoto? 此时外部的Photo3D已经生成, 仅需要手动调用Photo3D->RecordProperty(), 性能较差.
	2. OnCopyAfterPlacedByPhoto? 此时外部Photo3D已被放置, 收起再复制会影响回溯, 复制后再收起?
	3. (使用中)新增OnCopyBeginPlacedByPhoto.

未修复BUG: 同时删除(关卡实例 && (Texture2D | MIC))时, 编辑器会无响应.
*/

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VFPHOTOCATCHEREDITOR_API AVFPhotoCatcherPref : public AVFPhotoCatcher
{
	GENERATED_BODY()

public:
	AVFPhotoCatcherPref();

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform &Transform) override;

	virtual TArray<UPrimitiveComponent *> GetOverlapComps_Implementation() override;

	// 你可以用视锥收集后, 再手动删除不需要的Actor. 减少工作量.
	UFUNCTION(CallInEditor, Category = "ViewFinder")
	void RecollectShowOnlyActors();

	UFUNCTION(CallInEditor, Category = "ViewFinder")
	void PrefabricateAPhotoLevel();

	// 手动迭代, 避开同时读写造成的黑, 也是从版本兼容性考虑
	UFUNCTION(CallInEditor, Category = "ViewFinder")
	void UpdateMIC();

	virtual AVFPhoto2D *TakeAPhoto_Implementation() override;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "ViewFinder")
	FName TextureName = TEXT("Texture");

	UPROPERTY(EditAnywhere, Category = "ViewFinder")
	int MaterialIndex = 0;

	// 若不为空, 则只会对里面Actors进行处理.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TArray<TObjectPtr<AActor>> OnlyActorsCatched;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<UMaterialInstanceConstant> MIConstantAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<UTexture2D> Texture2DAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<UMaterialInstanceConstant> BgMIConstantAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	TObjectPtr<UTexture2D> BgTexture2DAsset;
#endif
#endif
};
