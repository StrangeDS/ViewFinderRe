#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFHelperComponent.generated.h"

UENUM(BlueprintType)
enum class FVFShowInPhotoRule : uint8
{
	Neither,
	OriginalOnly,
	CopyOnly,
	Both,
	MAX
};

UENUM(BlueprintType)
enum class FVFHelperDelegateType : uint8
{
	OriginalBeforeCheckVFDMComps,
	OriginalBeforeBeingCopied,
	OriginalBeforeBegingCut,
	OriginalBeforeTakingPhoto,
	OriginalEndTakingPhoto,
	OriginalEndPlacingPhoto,
	CopyBeforeBeingCut,
	CopyBeforeTakingPhoto,
	CopyBeforeFoldedInPhoto,
	CopyEndTakingPhoto,
	CopyBeforeBeingEnabled,
	CopyEndPlacingPhoto,
	MAX
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

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	/*
	默认被拍照的是生成的副本, 没有进入拍照流程的自然没有副本.
	但又相对独立, 可以设置原本和副本的被捕获情况.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	FVFShowInPhotoRule ShowInPhotoRule = FVFShowInPhotoRule::CopyOnly;

	// 能否被拍入照片(进入后续的复制等流程)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bCanBeTakenInPhoto = true;

	// 能否被放置的照片覆盖(进入后续的差集流程)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bCanBePlacedByPhoto = true;

	// 是否在被拍照时使用替身
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bReplacedWithStandIn = false;

	// 使用替身时, 子Actors是否也无视
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bIgnoreChildActors = true;

	// 需具体实现的替身类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder",
			  meta = (EditCondition = "bReplacedWithStandIn"))
	TSubclassOf<AActor> StandInClass;

	// 使用替身的Actor, 便于访问使用
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "ViewFinder",
			  meta = (EditCondition = "bReplacedWithStandIn", NoEditInline))
	TObjectPtr<AActor> ActorStandInFor;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	bool NotifyDelegate(UObject *Sender, const FVFHelperDelegateType &Type);

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnOriginalBeforeCheckVFDMComps;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnOriginalBeforeBeingCopied;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnOriginalBeforeBegingCut;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnOriginalBeforeTakingPhoto;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnOriginalEndTakingPhoto;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnOriginalEndPlacingPhoto;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnCopyBeforeBeingCut;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnCopyBeforeTakingPhoto;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnCopyBeforeFoldedInPhoto;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnCopyEndTakingPhoto;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnCopyBeforeBeingEnabled;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnCopyEndPlacingPhoto;
};
