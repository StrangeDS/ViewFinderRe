#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFHelperComponent.generated.h"

UENUM(BlueprintType)
enum class FVFHelperDelegateType : uint8
{
	OriginalBeforeTakingPhoto,
	OriginalBeforeCheckVFDMComps,
	OriginalBeforeBeingCopied,
	OriginalBeforeBegingCut,
	OriginalEndTakingPhoto,
	OriginalEndPlacingPhoto,
	CopyBeforeBeingCut,
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
	能否在照片中显示, 与bCanBeTakenInPhoto独立.
	意味着可以: 显示但不进入后续流程, 不显示但进入后续流程.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bCanShowInPhoto = true;

	// 能否被拍入照片(进入后续的复制等流程)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bCanBeTakenInPhoto = true;

	// 能否被放置的照片覆盖(差集)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bCanBePlacedByPhoto = true;

	// 是否使用替身
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bReplacedWithStandIn = false;

	// 使用替身时, 子Actors也被无视
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bIgnoreChildActors = true;

	// 替身类, 需实现
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder", meta = (EditCondition = "bReplacedWithStandIn"))
	TSubclassOf<AActor> StandInClass;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	bool NotifyDelegate(UObject *Sender, const FVFHelperDelegateType &Type);

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnOriginalBeforeTakingPhoto;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnOriginalBeforeCheckVFDMComps;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnOriginalBeforeBeingCopied;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnOriginalBeforeBegingCut;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnOriginalEndTakingPhoto;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnOriginalEndPlacingPhoto;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnCopyBeforeBeingCut;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnCopyBeforeFoldedInPhoto;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnCopyEndTakingPhoto;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnCopyBeforeBeingEnabled;

	UPROPERTY(BlueprintAssignable, Category = "ViewFinder")
	FVFHelperDelegate OnCopyEndPlacingPhoto;
};
