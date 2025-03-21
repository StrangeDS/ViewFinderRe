#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFHelperComponent.generated.h"

UENUM(BlueprintType)
enum class FVFHelperDelegateType : uint8
{
	OriginalBeforeTakenInPhoto,
	OriginalBeforeCopyingToPhoto,
	OriginalAfterCutByPhoto,
	OriginalAfterTakingPhoto,
	CopyAfterCopiedForPhoto,
	CopyBeforeFoldedInPhoto,
	CopyAfterPlacedByPhoto
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVFHelperDelegate, UObject *, Sender);

UCLASS(Blueprintable, ClassGroup = (ViewFinder), meta = (BlueprintSpawnableComponent))
class VIEWFINDERCORE_API UVFHelperComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVFHelperComponent(const FObjectInitializer &ObjectInitializer = FObjectInitializer::Get());

public:
	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;

public: // 注意: PROPERTY在CloneActorRuntime中不会被复制. 注意默认值.
	// 能否被拍入照片
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bCanBeTakenInPhoto = true;

	// 能否被放置的照片覆盖(差集)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bCanBePlacedByPhoto = true;

	// 是否使用替身
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bReplacedWithStandIn = false;

	// 替身类, 需实现
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder", meta = (EditCondition = "bReplacedWithStandIn"))
	TSubclassOf<AActor> StandInClass;

public:
	bool NotifyDelegate(UObject *Sender, const FVFHelperDelegateType &Type);

	// 原Actor(Original)
	// 衍生出VFDMComp之前, Sender为PhotoCatcher
	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnOriginalBeforeTakenInPhoto;

	// 复制对应Actor之前, Sender为PhotoCatcher
	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnOriginalBeforeCopyingToPhoto;

	// 被相片覆盖裁剪之后. 相机bCuttingOriginal为true才会触发, Sender为PhotoCatcher
	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnOriginalAfterCutByPhoto;

	// (Photo2D)拍照完成后, Sender为 PhotoCatcher
	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnOriginalAfterTakingPhoto;

	// 复制出的Actor(Copy)
	// 复制后, Sender为 PhotoCatcher 或 Photo3D
	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnCopyAfterCopiedForPhoto;

	// Photo3D折叠前, Sender为Photo3D
	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnCopyBeforeFoldedInPhoto;

	// Photo3D放置后, Sender为Photo3D
	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnCopyAfterPlacedByPhoto;
};
