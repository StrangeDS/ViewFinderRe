// Copyright 2026, StrangeDS. All Rights Reserved.

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
class VFPHOTOCOMMON_API UVFHelperComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVFHelperComponent(const FObjectInitializer &ObjectInitializer = FObjectInitializer::Get());

public:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext &Context) const override;
#endif

public:
	/*
	Configurable whether to be captured for originals and duplicates.
	By default, the duplicates are captured.
	Actors without Helpers are captured by default.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	FVFShowInPhotoRule ShowInPhotoRule = FVFShowInPhotoRule::CopyOnly;

	// whether to enter photo-taking process.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bCanBeTakenInPhoto = true;

	// whether to enter photo3d placing process.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bCanBePlacedByPhoto = true;

	// whether to use StandIn in photo-taking process.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bReplacedWithStandIn = false;

	// whether to ignore children actors when using StandIn.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder")
	bool bIgnoreChildActors = true;

	// StandIn class to use.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewFinder",
			  meta = (EditCondition = "bReplacedWithStandIn",
					  MustImplement = "/Script/VFPhotoCommon.VFStandInInterface"))
	TSubclassOf<AActor> StandInClass;

	// What Actor is standing for. Not available to Blueprint Class.
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