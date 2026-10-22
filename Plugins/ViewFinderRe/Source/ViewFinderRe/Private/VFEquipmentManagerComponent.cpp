// Copyright 2026. All Rights Reserved.

#include "VFEquipmentManagerComponent.h"

#include "Engine/World.h"

#include "VFStepsRecorderWorldSubsystem.h"

UVFEquipmentManagerComponent::UVFEquipmentManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UVFEquipmentManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    if (auto *Recorder = GetWorld()->GetSubsystem<UVFStepsRecorderWorldSubsystem>())
    {
        Recorder->OnEndRewinding.AddUniqueDynamic(this, &ThisClass::OnEndRewinding);
    }
}

void UVFEquipmentManagerComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    if (GetWorld())
    {
        if (auto *Recorder = GetWorld()->GetSubsystem<UVFStepsRecorderWorldSubsystem>())
        {
            Recorder->OnEndRewinding.RemoveDynamic(this, &ThisClass::OnEndRewinding);
        }
    }
    DeactivateCurEquipment();
    Equipments.Reset();
    EquipmentCurIndex = INDEX_NONE;
    Super::EndPlay(Reason);
}

void UVFEquipmentManagerComponent::AddEquipment(const TScriptInterface<IVFActivatableInterface> &Equipment)
{
    if (IsValid(Equipment.GetObject()) && Equipment.GetObject()->Implements<UVFActivatableInterface>())
    {
        Equipments.AddUnique(Equipment);
    }
}

void UVFEquipmentManagerComponent::RemoveEquipment(const TScriptInterface<IVFActivatableInterface> &Equipment)
{
    const int32 Index = Equipments.Find(Equipment);
    if (Index == INDEX_NONE)
    {
        return;
    }
    if (EquipmentCurIndex == Index)
    {
        DeactivateCurEquipment();
        EquipmentCurIndex = INDEX_NONE;
    }
    else if (EquipmentCurIndex > Index)
    {
        --EquipmentCurIndex;
    }
    Equipments.RemoveAt(Index);
}

bool UVFEquipmentManagerComponent::SwitchEquipment(const TScriptInterface<IVFActivatableInterface> &Equipment)
{
    const int32 Index = Equipments.Find(Equipment);
    UObject *Object = Equipment.GetObject();
    if (Index == INDEX_NONE || !IsValid(Object) || !IVFActivatableInterface::Execute_CanActivate(Object))
    {
        return false;
    }
    DeactivateCurEquipment();
    EquipmentCurIndex = INDEX_NONE;
    if (!IVFActivatableInterface::Execute_TryActivate(Object))
    {
        return false;
    }
    EquipmentCurIndex = Index;
    return true;
}

void UVFEquipmentManagerComponent::SwitchEquipmentNext()
{
    const int32 Start = EquipmentCurIndex + 1;
    DeactivateCurEquipment();
    EquipmentCurIndex = INDEX_NONE;
    for (int32 Index = Start; Index < Equipments.Num(); ++Index)
    {
        if (SwitchEquipment(Equipments[Index]))
        {
            return;
        }
    }
}

void UVFEquipmentManagerComponent::DeactivateCurEquipment()
{
    if (!Equipments.IsValidIndex(EquipmentCurIndex))
    {
        return;
    }
    UObject *Object = Equipments[EquipmentCurIndex].GetObject();
    if (IsValid(Object) && IVFActivatableInterface::Execute_IsActive(Object))
    {
        IVFActivatableInterface::Execute_Deactivate(Object);
    }
}

void UVFEquipmentManagerComponent::OnEndRewinding(float Time)
{
    EquipmentCurIndex = INDEX_NONE;
    for (int32 Index = 0; Index < Equipments.Num(); ++Index)
    {
        if (IsValid(Equipments[Index].GetObject()) &&
            IVFActivatableInterface::Execute_IsActive(Equipments[Index].GetObject()))
        {
            EquipmentCurIndex = Index;
            break;
        }
    }
}
