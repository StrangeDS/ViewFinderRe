#include "VFPhotoContainer.h"

#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

#include "VFPhoto2D.h"
#include "VFHelperComponent.h"
#include "VFFunctions.h"

AVFPhotoContainer::AVFPhotoContainer()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	Container = CreateDefaultSubobject<USceneComponent>(TEXT("Container"));
	Container->SetupAttachment(RootComponent);
	Container->SetRelativeLocation(FVector(100.0f, -50.0f, 0.f));
	Container->SetRelativeRotation(FRotator(-30.0f, -30.0f, 0.f));

	Preview = CreateDefaultSubobject<USceneComponent>(TEXT("Preview"));
	Preview->SetRelativeLocation(FVector(59.0f, 0.f, 0.f)); // 需对齐AVFPhotoCatcher_PickUp::PreviewTrans
	Preview->SetupAttachment(RootComponent);

	Helper = CreateDefaultSubobject<UVFHelperComponent>(TEXT("Helper"));
	Helper->bCanBeTakenInPhoto = false;
	Helper->bCanBePlacedByPhoto = false;
}

void AVFPhotoContainer::AddAPhoto(AVFPhoto2D *Photo)
{
	check(Photo);

	if (CurrentPhoto2D)
		CurrentPhoto2D->SetActorHiddenInGame(true);

	Photo->SetActorEnableCollision(false);
	Photo2Ds.EmplaceLast(Photo);

	UpdateCurrentPhoto();
}

void AVFPhotoContainer::PrepareCurrentPhoto()
{
	if (!CurrentPhoto2D)
		return;

	PlayerController->GetPawn()->DisableInput(PlayerController);

	GetWorldTimerManager().ClearTimer(PrepareTimeHandle);
	GetWorldTimerManager().SetTimer(
		PrepareTimeHandle,
		this,
		&AVFPhotoContainer::PrepareCurrentPhoto_Move,
		PrepareMoveInterval,
		true);
}

void AVFPhotoContainer::GiveUpPreparing()
{
	if (!CurrentPhoto2D)
		return;

	bFocusOn = false;
	PlayerController->GetPawn()->EnableInput(PlayerController);
	CurrentPhoto2D->Preview(Preview->GetComponentToWorld(), false);
	Preview->SetRelativeRotation(FRotator::ZeroRotator);

	GetWorldTimerManager().ClearTimer(PrepareTimeHandle);
	GetWorldTimerManager().SetTimer(
		PrepareTimeHandle,
		this,
		&AVFPhotoContainer::GiveUpPreparing_Move,
		PrepareMoveInterval,
		true);
}

void AVFPhotoContainer::PlaceCurrentPhoto()
{
	if (!CurrentPhoto2D)
		return;

	CurrentPhoto2D->ReattachToComponent(nullptr);
	CurrentPhoto2D->PlaceDown();
	GiveUpPreparing();
	Photo2Ds.PopLast();
	UpdateCurrentPhoto();
}

void AVFPhotoContainer::ChangeCurrentPhoto(const bool Next)
{
	if (Photo2Ds.Num() <= 1)
		return;

	if (bFocusOn)
		return;

	if (CurrentPhoto2D)
		CurrentPhoto2D->SetActorHiddenInGame(true);

	if (Next)
	{
		auto Photo = Photo2Ds.First();
		Photo2Ds.PopFirst();
		Photo2Ds.EmplaceLast(Photo);
	}
	else
	{
		auto Photo = Photo2Ds.Last();
		Photo2Ds.PopLast();
		Photo2Ds.EmplaceFirst(Photo);
	}
	UpdateCurrentPhoto();
}

void AVFPhotoContainer::UpdateCurrentPhoto()
{
	GetWorldTimerManager().ClearTimer(PrepareTimeHandle);

	CurrentPhoto2D = Photo2Ds.IsEmpty() ? nullptr : Photo2Ds.Last();
	if (CurrentPhoto2D)
		CurrentPhoto2D->SetActorHiddenInGame(!bEnabled);
}

void AVFPhotoContainer::RotateCurrentPhoto(float Delta)
{
	if (!bFocusOn)
		return;

	CurrentPhoto2D->GetRootComponent()->AddRelativeRotation(FRotator(0.f, 0.f, Delta * RotateFactor));
	CurrentPhoto2D->Preview(CurrentPhoto2D->GetActorTransform(), true);
}

void AVFPhotoContainer::AlignCurrentPhoto()
{
	if (!bFocusOn)
		return;

	auto Rotation = PlayerController->GetControlRotation();
	Rotation.Pitch = 0.f;
	PlayerController->SetControlRotation(Rotation);
}

void AVFPhotoContainer::SetEnabled(const bool &EnabledIn)
{
	if (bEnabled == EnabledIn)
		return;
	bEnabled = EnabledIn;

	check(PlayerController);

	UpdateCurrentPhoto();
	SetActorHiddenInGame(!bEnabled);

	OnEnabled.Broadcast(bEnabled);
}

void AVFPhotoContainer::PrepareCurrentPhoto_Move()
{
	auto TransCur = CurrentPhoto2D->ActorToWorld();
	auto TransTarget = Preview->GetComponentToWorld();
	bFocusOn = TransCur.EqualsNoScale(TransTarget);
	if (bFocusOn)
	{
		GetWorldTimerManager().ClearTimer(PrepareTimeHandle);

		CurrentPhoto2D->Preview(TransTarget, true);
	}
	else
	{
		auto Rate = GetWorldTimerManager().GetTimerRate(PrepareTimeHandle);
		Rate = 1 - Rate / TimeOfPrepare;
		auto TransNext = UVFFunctions::TransformLerpNoScale(TransCur, TransTarget, Rate);
		CurrentPhoto2D->SetActorTransform(TransNext);
	}
}

void AVFPhotoContainer::GiveUpPreparing_Move()
{
	auto TransCur = CurrentPhoto2D->ActorToWorld();
	auto TransTarget = Container->GetComponentToWorld();
	if (TransCur.EqualsNoScale(TransTarget))
	{
		GetWorldTimerManager().ClearTimer(PrepareTimeHandle);
	}
	else
	{
		auto Rate = GetWorldTimerManager().GetTimerRate(PrepareTimeHandle);
		Rate = 1 - Rate / TimeOfGivingUp;
		auto TransNext = UVFFunctions::TransformLerpNoScale(TransCur, TransTarget, Rate);
		CurrentPhoto2D->SetActorTransform(TransNext);
	}
}

int AVFPhotoContainer::GetPhoto2DNum_Implementation()
{
	return Num();
}

bool AVFPhotoContainer::TakeIn_Implementation(AVFPhoto2D *Photo2D, const bool &Enabled)
{
	Photo2D->ReattachToComponent(Container);
	AddAPhoto(Photo2D);
	return true;
}