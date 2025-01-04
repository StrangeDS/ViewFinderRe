#include "VFPhotoContainer.h"

#include "Kismet/GameplayStatics.h"
// #include "EnhancedInputSubsystems.h"
// #include "InputMappingContext.h"

#include "VFPhoto2D.h"
#include "VFHelperComponent.h"

AVFPhotoContainer::AVFPhotoContainer()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	Container = CreateDefaultSubobject<USceneComponent>(TEXT("Container"));
	Container->SetupAttachment(RootComponent);

	// static ConstructorHelpers::FObjectFinder<UInputMappingContext> Selector(
	// 	TEXT("/Game/ViewFinder/Input/PhotoContainer/IMC_PhotoContainer.IMC_PhotoContainer"));
	// MappingContext = Selector.Object;

	Helper = CreateDefaultSubobject<UVFHelperComponent>(TEXT("Helper"));
	Helper->bCanBeTakenInPhoto = false;
	Helper->bCanBePlacedByPhoto = false;
}

void AVFPhotoContainer::BeginPlay()
{
	Super::BeginPlay();
}

void AVFPhotoContainer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AVFPhotoContainer::AddAPhoto(AVFPhoto2D *Photo)
{
	check(Photo);

	Photo2Ds.EmplaceLast(Photo);
	Photo->AttachToComponent(Container, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	UpdateCurrentPhoto();
}

void AVFPhotoContainer::PrepareCurrentPhoto(float Time)
{
	if (!CurrentPhoto2D)
		return;

	Time = Time >= 0.f ? Time : TimeOfSelect;
	PlayerController->GetPawn()->DisableInput(PlayerController);
	GetWorldTimerManager().SetTimer(
		PrepareTimeHandle, [this]()
		{
            CurrentPhoto2D->Preview(GetActorTransform(), true);
            bFocusOn = true; },
		Time,
		false);
}

void AVFPhotoContainer::GiveUpPreparing()
{
	if (!CurrentPhoto2D)
	 	return;

	GetWorldTimerManager().ClearTimer(PrepareTimeHandle);
	CurrentPhoto2D->Preview(GetActorTransform(), false);
	bFocusOn = false;
	PlayerController->GetPawn()->EnableInput(PlayerController);
}

void AVFPhotoContainer::PlaceCurrentPhoto()
{
	if (!CurrentPhoto2D)
	 	return;

	CurrentPhoto2D->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	CurrentPhoto2D->PlaceDown();
	Photo2Ds.PopLast();
	GiveUpPreparing();
	UpdateCurrentPhoto();
}

void AVFPhotoContainer::ChangeCurrentPhoto(const bool Next)
{
	if (Photo2Ds.Num() <= 1)
		return;

	if (bFocusOn)
		return;

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
	if (CurrentPhoto2D)
		CurrentPhoto2D->SetActorHiddenInGame(true);

	CurrentPhoto2D = Photo2Ds.IsEmpty() ? nullptr : Photo2Ds.Last();

	if (CurrentPhoto2D)
	{
		if (bEnabled)
			CurrentPhoto2D->SetActorHiddenInGame(false);
	}
	else
	{
		SetEnabled(false);
	}
}

void AVFPhotoContainer::SetEnabled(const bool &Enabled)
{
	if (bEnabled == Enabled)
		return;
	bEnabled = Enabled;

	if (Num() <= 0)
		bEnabled = false;

	check(PlayerController);

	if (bEnabled)
	{
		// auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		// if (Subsystem)
		// 	Subsystem->AddMappingContext(MappingContext, 2);
		// EnableInput(PlayerController);
		SetActorHiddenInGame(false);
		UpdateCurrentPhoto();
	}
	else
	{
		// DisableInput(PlayerController);
		// auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		// if (Subsystem)
		// 	Subsystem->RemoveMappingContext(MappingContext);
		SetActorHiddenInGame(true);
		UpdateCurrentPhoto();
	}
	OnEnabled.Broadcast(Enabled);
}
