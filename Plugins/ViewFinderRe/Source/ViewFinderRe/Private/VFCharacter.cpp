// Copyright StrangeDS. All Rights Reserved.

#include "VFCharacter.h"

#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Kismet/KismetSystemLibrary.h"

#include "VFLog.h"
#include "VFInteractInterface.h"
#include "VFHelperComponent.h"
#include "VFPawnStandIn.h"
#include "VFStepsRecorderWorldSubsystem.h"
#include "VFActivatableInterface.h"
#include "VFPhotoContainer_Input.h"

FVFPawnTransformInfo::FVFPawnTransformInfo(APawn *Pawn, float TimeIn)
	: Location(Pawn->GetActorLocation()),
	  Velocity(Pawn->GetVelocity()),
	  Rotator(Pawn->GetViewRotation()),
	  Time(TimeIn)
{
}

FVFPawnTransformInfo::FVFPawnTransformInfo(const FVFPawnTransformInfo &Other, float TimeIn)
	: Location(Other.Location),
	  Velocity(Other.Velocity),
	  Rotator(Other.Rotator),
	  Time(TimeIn)
{
}

bool FVFPawnTransformInfo::operator==(const FVFPawnTransformInfo &Other) const
{
	if (Location != Other.Location)
		return false;
	if (Velocity != Other.Velocity)
		return false;
	return Rotator == Other.Rotator;
}

AVFCharacter::AVFCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
	Camera->SetRelativeLocation(FVector(-10.f, 0.f, 60.f));
	Camera->bUsePawnControlRotation = true;

	Helper = CreateDefaultSubobject<UVFHelperComponent>("Helper");
	Helper->ShowInPhotoRule = FVFShowInPhotoRule::Neither;
	Helper->bCanBePlacedByPhoto = false;
	Helper->bReplacedWithStandIn = true;
	Helper->StandInClass = AVFPawnStandIn::StaticClass();

	ContainerClass = AVFPhotoContainer_Input::StaticClass();
}

void AVFCharacter::BeginPlay()
{
	Super::BeginPlay();

	Container = GetWorld()->SpawnActor<AVFPhotoContainer_Input>(ContainerClass);
	Container->AttachToComponent(Camera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	if (IsValid(PlayerController))
		Container->SetPlayerController(PlayerController);
	Equipments.Add(Container);

	if (!GetStepsRecorder())
	{
		VF_LOG(Error, TEXT("%s invalid StepsRecorder."), __FUNCTIONW__);
		return;
	}
	Steps.Reserve(UVFStepsRecorderWorldSubsystem::GetSizeRecommended());
	StepsRecorder->RegisterTickable(this);
	StepsRecorder->OnEndRewinding.AddUniqueDynamic(this, &AVFCharacter::OnEndRewinding);
}

void AVFCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TryToTraceInteractable(DeltaTime);
}

void AVFCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent *EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AVFCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AVFCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AVFCharacter::Interact);
		EnhancedInputComponent->BindAction(SwitchAction, ETriggerEvent::Started, this, &AVFCharacter::SwitchEquipmentNext);
	}
}

void AVFCharacter::PossessedBy(AController *NewController)
{
	Super::PossessedBy(NewController);

	PlayerController = Cast<APlayerController>(Controller);
	if (!IsValid(PlayerController))
	{
		VF_LOG(Warning, TEXT("%s: PlayerController is invalid."), __FUNCTIONW__);
		return;
	}
	if (!IsValid(PlayerController->GetLocalPlayer()))
	{
		VF_LOG(Warning, TEXT("%s: GetLocalPlayer is invalid."), __FUNCTIONW__);
		return;
	}

	if (IsValid(Container))
		Container->SetPlayerController(PlayerController);

	if (UEnhancedInputLocalPlayerSubsystem *Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(MappingContext, 0);
	}
}

void AVFCharacter::UnPossessed()
{
	if (UEnhancedInputLocalPlayerSubsystem *Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->RemoveMappingContext(MappingContext);
	}
	PlayerController = nullptr;

	Super::UnPossessed();
}

void AVFCharacter::TryToTraceInteractable(float DeltaTime)
{
	TimeSinceLastTrace += DeltaTime;
	bool NeedTrace = false;
	while (TimeSinceLastTrace > TraceInterval)
	{
		NeedTrace = true;
		TimeSinceLastTrace -= TraceInterval;
	}
	if (NeedTrace)
		TraceInteractable();
}

void AVFCharacter::TraceInteractable()
{
	FHitResult OutHit;
	FVector From = Camera->GetComponentLocation();
	FVector To = Camera->GetComponentQuat().GetForwardVector() * TraceDistance;
	To = From + To;
	bool Traced = UKismetSystemLibrary::LineTraceSingle(
		this,
		From,
		To,
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		{},
		EDrawDebugTrace::ForDuration,
		OutHit,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		0.5f);

	auto Object = InteractingObject.GetObject();
	auto Actor = OutHit.GetActor();
	auto Component = OutHit.GetComponent();
	if (Object == Actor || Object == Component)
	{
		return;
	}

	if (IsValid(Object))
		IVFInteractInterface::Execute_EndAiming(Object, PlayerController);

	if (!Traced)
	{
		InteractingObject = nullptr;
		return;
	}

	if (IsValid(Actor) && Actor->Implements<UVFInteractInterface>())
	{
		InteractingObject = Actor;
		IVFInteractInterface::Execute_StartAiming(Actor, PlayerController);
	}
	else if (IsValid(Component) && Component->Implements<UVFInteractInterface>())
	{
		InteractingObject = Component;
		IVFInteractInterface::Execute_StartAiming(Component, PlayerController);
	}
	else
	{
		InteractingObject = nullptr;
	}
}

void AVFCharacter::Move(const FInputActionValue &Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (IsValid(Controller))
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AVFCharacter::Look(const FInputActionValue &Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (IsValid(Controller))
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AVFCharacter::Interact()
{
	if (auto Object = InteractingObject.GetObject(); IsValid(Object))
	{
		IVFInteractInterface::Execute_Interact(Object, PlayerController);
	}
}

void AVFCharacter::TickForward_Implementation(float Time)
{
	const static float Permitted = 0.1f;

	FVFPawnTransformInfo Info(this, StepsRecorder->Time);
	if (Steps.IsEmpty())
	{
		Steps.Add(Info);
		return;
	}

	if (Steps.Last() != Info)
	{
		/*
		For teleportation, the following solution is adopted:
		The time difference for continuous movement should be < PermittedValue.
		Teleportation is treated as movement after prolonged inactivity,
		requiring data to be resubmitted once more immediately before the teleport occurs.
		*/
		auto LastInfo = Steps.Last();
		if (Time - LastInfo.Time > Permitted)
		{
			FVFPawnTransformInfo RepeatInfo(LastInfo, StepsRecorder->Time);
			Steps.Add(RepeatInfo);
		}

		Steps.Add(Info);
	}
}

void AVFCharacter::TickBackward_Implementation(float Time)
{
	while (Steps.Num() > 1)
	{
		auto &StepInfo = Steps.Last();
		if (StepInfo.Time < Time)
			break;

		SetActorLocation(StepInfo.Location);
		if (GetRootComponent() && GetRootComponent()->IsSimulatingPhysics())
			GetRootComponent()->ComponentVelocity = StepInfo.Velocity;
		else
			GetCharacterMovement()->Velocity = StepInfo.Velocity;
		GetController()->SetControlRotation(StepInfo.Rotator);

		Steps.Pop(false);
	}

	auto &Step = Steps.Last();
	if (Time < Step.Time)
		return;

	auto Delta = StepsRecorder->GetDeltaTime() / (StepsRecorder->GetTime() - Step.Time);
	Delta = FMath::Min(Delta, 1.0f);
	SetActorLocation(FMath::Lerp(GetActorLocation(), Step.Location, Delta));

	auto Velocity = FMath::Lerp(GetVelocity(), Step.Velocity, Delta);
	if (GetRootComponent() && GetRootComponent()->IsSimulatingPhysics())
	{
		GetRootComponent()->ComponentVelocity = Velocity;
	}
	else
	{
		GetCharacterMovement()->Velocity = Velocity;
	}

	GetController()->SetControlRotation(FMath::Lerp(GetViewRotation(), Step.Rotator, Delta));
}

int AVFCharacter::GetPhoto2DNum_Implementation()
{
	if (IsValid(Container) && Container->Implements<UVFPhotoContainerInterface>())
	{
		return IVFPhotoContainerInterface::Execute_GetPhoto2DNum(Container);
	}
	return -1;
}

bool AVFCharacter::TakeIn_Implementation(AVFPhoto2D *Photo2D, const bool &Enabled)
{
	if (IsValid(Container) && Container->Implements<UVFPhotoContainerInterface>())
	{
		if (IVFPhotoContainerInterface::Execute_TakeIn(Container, Photo2D))
		{
			if (Container->Num() == 1)
				SwitchEquipment(Container);
			return true;
		}
	}
	return false;
}

UVFHelperComponent *AVFCharacter::GetHelper_Implementation()
{
	return Helper;
}

void AVFCharacter::SwitchEquipmentNext_Implementation()
{
	DeactivateCurEquipment();
	if (EquipmentCurIndex == Equipments.Num() - 1)
	{
		EquipmentCurIndex = INDEX_NONE;
		return;
	}

	for (EquipmentCurIndex = EquipmentCurIndex + 1; EquipmentCurIndex < Equipments.Num(); ++EquipmentCurIndex)
	{
		if (IVFActivatableInterface::Execute_CanActivate(Equipments[EquipmentCurIndex].GetObject()))
		{
			IVFActivatableInterface::Execute_Activate(Equipments[EquipmentCurIndex].GetObject());
			return;
		}
	}

	EquipmentCurIndex = INDEX_NONE;
}

void AVFCharacter::AddEquipment_Implementation(const TScriptInterface<IVFActivatableInterface> &Equipment)
{
	Equipments.AddUnique(Equipment);
}

void AVFCharacter::RemoveEquipment_Implementation(const TScriptInterface<IVFActivatableInterface> &Equipment)
{
	int Index = Equipments.Find(Equipment);
	if (EquipmentCurIndex == Index)
		DeactivateCurEquipment();
	Equipments.Remove(Equipment);
	if (EquipmentCurIndex >= Equipments.Num() - 1)
	{
		EquipmentCurIndex = INDEX_NONE;
	}
}

void AVFCharacter::DeactivateCurEquipment_Implementation()
{
	if (EquipmentCurIndex != INDEX_NONE)
	{
		auto Equipment = Equipments[EquipmentCurIndex].GetObject();
		if (IsValid(Equipment))
		{
			if (IVFActivatableInterface::Execute_IsActive(Equipment))
				IVFActivatableInterface::Execute_Deactivate(Equipment);
		}
	}
}

bool AVFCharacter::SwitchEquipment_Implementation(const TScriptInterface<IVFActivatableInterface> &Equipment)
{
	if (!Equipments.Contains(Equipment))
		return false;
	DeactivateCurEquipment();
	EquipmentCurIndex = Equipments.Find(Equipment);
	return IVFActivatableInterface::Execute_TryActivate(Equipment.GetObject());
}

void AVFCharacter::OnEndRewinding(float Time)
{
	/*
	The device automatically rewinds its enabled state.
	Here we verify and update EquipmentCurIndex.
	*/
	for (auto Equipment : Equipments)
	{
		if (IVFActivatableInterface::Execute_IsActive(Equipment.GetObject()))
		{
			EquipmentCurIndex = Equipments.Find(Equipment);
			return;
		}
	}
	EquipmentCurIndex = INDEX_NONE;
}