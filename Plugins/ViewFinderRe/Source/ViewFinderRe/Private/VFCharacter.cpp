#include "VFCharacter.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

#include "Kismet/KismetSystemLibrary.h"

#include "VFInteractInterface.h"
#include "VFPhotoContainer.h"
#include "VFHelperComponent.h"
#include "VFPawnStandIn.h"

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
	Helper->bCanBePlacedByPhoto = false;
	Helper->bReplacedWithStandIn = true;
	Helper->StandInClass = AVFPawnStandIn::StaticClass();
}

void AVFCharacter::BeginPlay()
{
	Super::BeginPlay();

	StepRecorder = GetWorld()->GetSubsystem<UVFStepsRecorderWorldSubsystem>();
	check(StepRecorder);
	StepRecorder->RegisterTickable(this);

	Container = GetWorld()->SpawnActor<AVFPhotoContainer>(ContainerClass);
	Container->AttachToComponent(Camera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	if (PlayerController)
		Container->SetPlayerController(PlayerController);
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
		EnhancedInputComponent->BindAction(SwitchAction, ETriggerEvent::Started, this, &AVFCharacter::Switch);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AVFCharacter::BeginDestroy()
{
	Super::BeginDestroy();
}

void AVFCharacter::PossessedBy(AController *NewController)
{
	Super::PossessedBy(NewController);

	PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController 不存在"));
		return;
	}
	if (!PlayerController->GetLocalPlayer())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetLocalPlayer 不存在"));
		return;
	}

	if (Container)
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

	if (Object)
		IVFInteractInterface::Execute_EndAiming(Object, PlayerController);

	if (!Traced)
	{
		InteractingObject = nullptr;
		return;
	}

	if (Actor->Implements<UVFInteractInterface>())
	{
		InteractingObject = Actor;
		IVFInteractInterface::Execute_StartAiming(Actor, PlayerController);
	}
	else if (Component->Implements<UVFInteractInterface>())
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
	if (Controller)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AVFCharacter::Look(const FInputActionValue &Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AVFCharacter::Interact()
{
	if (auto Object = InteractingObject.GetObject())
	{
		IVFInteractInterface::Execute_Interact(Object, PlayerController);
	}
}

void AVFCharacter::Switch()
{
	UE_LOG(LogTemp, Warning, TEXT("AVFCharacter::Switch"));
	Container->SetEnabled(!Container->IsEnabled());
}

void AVFCharacter::TickForward_Implementation(float Time)
{
	const static float Permitted = 0.1f;

	FVFPawnTransformInfo Info(this, StepRecorder->Time);
	if (Steps.IsEmpty())
	{
		Steps.Add(Info);
		return;
	}

	if (Steps.Last() != Info)
	{
		// 对于瞬移采取以下解决方案:
		// 连续运动时间差值应当 < Permitted,
		// 瞬移则是长时间不动后的移动, 在瞬移前再重复给入一次信息.
		auto LastInfo = Steps.Last();
		if (Time - LastInfo.Time > Permitted)
		{
			FVFPawnTransformInfo RepeatInfo(LastInfo, StepRecorder->Time);
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

		// 对于瞬移, 节点需要手动进行一次复位
		SetActorLocation(StepInfo.Location);
		if (GetRootComponent() && GetRootComponent()->IsSimulatingPhysics())
			GetRootComponent()->ComponentVelocity = StepInfo.Velocity;
		else
			GetCharacterMovement()->Velocity = StepInfo.Velocity;
		GetController()->SetControlRotation(StepInfo.Rotator);

		Steps.Pop(false);
	}

	auto &Step = Steps.Last();
	auto Delta = StepRecorder->GetDeltaTime() / (StepRecorder->GetTime() - Step.Time);
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
