// Copyright 2026. All Rights Reserved.
#include "VFPhotoContainerManagerComponent.h"
#include "VFPhotoContainer_Input.h"
#include "VFPhoto2D.h"
#include "VFEquipmentManagerComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
UVFPhotoContainerManagerComponent::UVFPhotoContainerManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
    ContainerClass = AVFPhotoContainer_Input::StaticClass();
}
void UVFPhotoContainerManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    APawn* Pawn = Cast<APawn>(GetOwner());
    if (!Pawn || !ContainerClass) return;
    FActorSpawnParameters Params;
    Params.Owner = Pawn;
    Container = GetWorld()->SpawnActor<AVFPhotoContainer_Input>(ContainerClass, Params);
    if (!IsValid(Container)) return;
    Container->AttachToActor(Pawn, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    if (auto* Camera = Pawn->FindComponentByClass<UCameraComponent>())
        Container->AttachToComponent(Camera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    else
        bFollowCameraManager = true;
    EquipmentManager = Pawn->FindComponentByClass<UVFEquipmentManagerComponent>();
    if (EquipmentManager) EquipmentManager->AddEquipment(Container.Get());
    Pawn->ReceiveControllerChangedDelegate.AddUniqueDynamic(this, &ThisClass::HandleControllerChanged);
    HandleControllerChanged(Pawn, nullptr, Pawn->GetController());
}
void UVFPhotoContainerManagerComponent::HandleControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
    if (!IsValid(Container)) return;
    if (IsValid(Container->PlayerController) && Container->IsEnabled())
        Container->SetEnabled(false);
    Container->SetPlayerController(Cast<APlayerController>(NewController));
    SetComponentTickEnabled(bFollowCameraManager && IsValid(Container->PlayerController));
}
void UVFPhotoContainerManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction)
{
    Super::TickComponent(DeltaTime, TickType, TickFunction);
    if (!bFollowCameraManager || !IsValid(Container) || !IsValid(Container->PlayerController)) return;
    if (auto* Camera = Container->PlayerController->PlayerCameraManager.Get())
        Container->SetActorLocationAndRotation(Camera->GetCameraLocation(), Camera->GetCameraRotation());
}
void UVFPhotoContainerManagerComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    if (auto* Pawn = Cast<APawn>(GetOwner()))
        Pawn->ReceiveControllerChangedDelegate.RemoveDynamic(this, &ThisClass::HandleControllerChanged);
    if (IsValid(Container))
    {
        if (IsValid(EquipmentManager)) EquipmentManager->RemoveEquipment(Container.Get());
        if (Container->IsEnabled() && IsValid(Container->PlayerController)) Container->SetEnabled(false);
        Container->Destroy();
        Container = nullptr;
    }
    Super::EndPlay(Reason);
}
int UVFPhotoContainerManagerComponent::GetPhoto2DNum_Implementation()
{
    return IsValid(Container) ? Container->Num() : 0;
}
bool UVFPhotoContainerManagerComponent::TakeIn_Implementation(AVFPhoto2D* Photo, const bool& Enabled)
{
    if (!IsValid(Photo) || !IsValid(Container) || !IsValid(Container->PlayerController)) return false;
    const bool Accepted = IVFPhotoContainerInterface::Execute_TakeIn(Container, Photo);
    if (Accepted && Enabled) HandlePhotoNumChanged(Container->Num());
    return Accepted;
}
void UVFPhotoContainerManagerComponent::HandlePhotoNumChanged(int Num)
{
    if (Num != 1 || !IsValid(Container) || !IsValid(Container->PlayerController)) return;
    if (IsValid(EquipmentManager)) EquipmentManager->SwitchEquipment(Container.Get());
    else IVFActivatableInterface::Execute_TryActivate(Container);
}
bool UVFPhotoContainerManagerComponent::TryTakeIn(APawn* Pawn, AVFPhoto2D* Photo, UObject** Recipient)
{
    if (Recipient) *Recipient = nullptr;
    if (!IsValid(Pawn) || !IsValid(Photo)) return false;
    if (Pawn->Implements<UVFPhotoContainerInterface>() &&
        IVFPhotoContainerInterface::Execute_TakeIn(Pawn, Photo))
    {
        if (Recipient) *Recipient = Pawn;
        return true;
    }
    if (auto* Manager = Pawn->FindComponentByClass<UVFPhotoContainerManagerComponent>())
        if (IVFPhotoContainerInterface::Execute_TakeIn(Manager, Photo))
        {
            if (Recipient) *Recipient = Manager->Container;
            return true;
        }
    TArray<AActor*> Attached;
    Pawn->GetAttachedActors(Attached);
    for (AActor* Actor : Attached)
    {
        if (!IsValid(Actor) || !Actor->Implements<UVFPhotoContainerInterface>()) continue;
        // A manager-owned container is handled above, including its readiness checks.
        if (auto* Manager = Pawn->FindComponentByClass<UVFPhotoContainerManagerComponent>())
            if (Actor == Manager->Container) continue;
        if (auto* ContainerActor = Cast<AVFPhotoContainer>(Actor))
            if (!IsValid(ContainerActor->PlayerController)) continue;
        if (!IVFPhotoContainerInterface::Execute_TakeIn(Actor, Photo)) continue;
        if (IVFPhotoContainerInterface::Execute_GetPhoto2DNum(Actor) == 1 && Actor->Implements<UVFActivatableInterface>())
            IVFActivatableInterface::Execute_TryActivate(Actor);
        if (Recipient) *Recipient = Actor;
        return true;
    }
    return false;
}
