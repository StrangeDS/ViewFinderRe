// Copyright 2026. All Rights Reserved.

#include "VFPhotoContainerManagerComponent.h"

#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#include "VFEquipmentManagerComponent.h"
#include "VFPhoto2D.h"
#include "VFPhotoContainer_Input.h"

UVFPhotoContainerManagerComponent::UVFPhotoContainerManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
    ContainerClass = AVFPhotoContainer_Input::StaticClass();
}

void UVFPhotoContainerManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    APawn *Pawn = Cast<APawn>(GetOwner());
    if (!Pawn || !ContainerClass)
    {
        return;
    }
    FActorSpawnParameters Params;
    Params.Owner = Pawn;
    Container = GetWorld()->SpawnActor<AVFPhotoContainer_Input>(ContainerClass, Params);
    if (!IsValid(Container))
    {
        return;
    }
    Container->AttachToActor(Pawn, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    if (auto *Camera = Pawn->FindComponentByClass<UCameraComponent>())
    {
        Container->AttachToComponent(Camera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    }
    else
    {
        bFollowCameraManager = true;
    }
    EquipmentManager = Pawn->FindComponentByClass<UVFEquipmentManagerComponent>();
    if (EquipmentManager)
    {
        EquipmentManager->AddEquipment(Container.Get());
    }
    Pawn->ReceiveControllerChangedDelegate.AddUniqueDynamic(this, &ThisClass::HandleControllerChanged);
    HandleControllerChanged(Pawn, nullptr, Pawn->GetController());
}

void UVFPhotoContainerManagerComponent::HandleControllerChanged(APawn *Pawn, AController *OldController,
                                                                AController *NewController)
{
    if (!IsValid(Container))
    {
        return;
    }
    if (IsValid(Container->PlayerController) && Container->IsEnabled())
    {
        Container->SetEnabled(false);
    }
    Container->SetPlayerController(Cast<APlayerController>(NewController));
    SetComponentTickEnabled(bFollowCameraManager && IsValid(Container->PlayerController));
}

void UVFPhotoContainerManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                      FActorComponentTickFunction *TickFunction)
{
    Super::TickComponent(DeltaTime, TickType, TickFunction);
    if (!bFollowCameraManager || !IsValid(Container) || !IsValid(Container->PlayerController))
    {
        return;
    }
    if (auto *Camera = Container->PlayerController->PlayerCameraManager.Get())
    {
        Container->SetActorLocationAndRotation(Camera->GetCameraLocation(), Camera->GetCameraRotation());
    }
}

void UVFPhotoContainerManagerComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    if (auto *Pawn = Cast<APawn>(GetOwner()))
    {
        Pawn->ReceiveControllerChangedDelegate.RemoveDynamic(this, &ThisClass::HandleControllerChanged);
    }
    if (IsValid(Container))
    {
        if (IsValid(EquipmentManager))
        {
            EquipmentManager->RemoveEquipment(Container.Get());
        }
        if (Container->IsEnabled() && IsValid(Container->PlayerController))
        {
            Container->SetEnabled(false);
        }
        Container->Destroy();
        Container = nullptr;
    }
    Super::EndPlay(Reason);
}

int UVFPhotoContainerManagerComponent::GetPhoto2DNum_Implementation()
{
    return IsValid(Container) ? Container->Num() : 0;
}

bool UVFPhotoContainerManagerComponent::TakeIn_Implementation(AVFPhoto2D *Photo, const bool &Enabled)
{
    if (!IsValid(Photo) || !IsValid(Container) || !IsValid(Container->PlayerController))
    {
        return false;
    }
    const bool bAccepted = IVFPhotoContainerInterface::Execute_TakeIn(Container, Photo);
    if (bAccepted && Enabled)
    {
        ActivateContainerForFirstPhoto();
    }
    return bAccepted;
}

void UVFPhotoContainerManagerComponent::ActivateContainerForFirstPhoto()
{
    // TakeIn is a Blueprint event, so recheck lifetime after it returns.
    if (!IsValid(Container) || !IsValid(Container->PlayerController) || Container->Num() != 1)
    {
        return;
    }

    if (IsValid(EquipmentManager))
    {
        EquipmentManager->SwitchEquipment(Container.Get());
    }
    else
    {
        IVFActivatableInterface::Execute_TryActivate(Container);
    }
}

bool UVFPhotoContainerManagerComponent::TryTakeInAttachedContainer(AActor *Actor, AVFPhoto2D *Photo)
{
    if (!Actor->Implements<UVFPhotoContainerInterface>())
    {
        return false;
    }

    if (auto *ContainerActor = Cast<AVFPhotoContainer>(Actor))
    {
        if (!IsValid(ContainerActor->PlayerController))
        {
            return false;
        }
    }

    if (!IVFPhotoContainerInterface::Execute_TakeIn(Actor, Photo))
    {
        return false;
    }

    if (IVFPhotoContainerInterface::Execute_GetPhoto2DNum(Actor) == 1 &&
        Actor->Implements<UVFActivatableInterface>())
    {
        IVFActivatableInterface::Execute_TryActivate(Actor);
    }
    return true;
}

bool UVFPhotoContainerManagerComponent::TryTakeIn(APawn *Pawn, AVFPhoto2D *Photo, UObject **OutRecipient)
{
    if (OutRecipient)
    {
        *OutRecipient = nullptr;
    }
    if (!IsValid(Pawn) || !IsValid(Photo))
    {
        return false;
    }

    if (Pawn->Implements<UVFPhotoContainerInterface>() &&
        IVFPhotoContainerInterface::Execute_TakeIn(Pawn, Photo))
    {
        if (OutRecipient)
        {
            *OutRecipient = Pawn;
        }
        return true;
    }

    auto *Manager = Pawn->FindComponentByClass<UVFPhotoContainerManagerComponent>();
    if (Manager && IVFPhotoContainerInterface::Execute_TakeIn(Manager, Photo))
    {
        if (OutRecipient)
        {
            *OutRecipient = Manager->Container;
        }
        return true;
    }

    TArray<AActor *> AttachedActors;
    Pawn->GetAttachedActors(AttachedActors);
    for (AActor *Actor : AttachedActors)
    {
        if (!IsValid(Actor))
        {
            continue;
        }

        // Do not bypass a manager's readiness checks by trying its attached container again.
        // Blueprint TakeIn implementations can change components, so resolve the manager again.
        Manager = Pawn->FindComponentByClass<UVFPhotoContainerManagerComponent>();
        if (Manager && Actor == Manager->Container)
        {
            continue;
        }

        if (TryTakeInAttachedContainer(Actor, Photo))
        {
            if (OutRecipient)
            {
                *OutRecipient = Actor;
            }
            return true;
        }
    }
    return false;
}
