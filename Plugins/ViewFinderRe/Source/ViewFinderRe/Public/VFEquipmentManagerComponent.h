// Copyright 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFActivatableInterface.h"
#include "VFEquipmentManagerComponent.generated.h"

/**
 * Selects one registered piece of equipment for a pawn that does not inherit AVFCharacter.
 * EquipmentCurIndex identifies the selection; INDEX_NONE means no selection.
 * Rewind restores equipment state separately, then OnEndRewinding reconciles the index.
 */
UCLASS(Blueprintable, ClassGroup = (ViewFinder), meta = (BlueprintSpawnableComponent))
class VIEWFINDERRE_API UVFEquipmentManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFEquipmentManagerComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;

    UFUNCTION(BlueprintCallable, Category = "ViewFinder")
    void AddEquipment(const TScriptInterface<IVFActivatableInterface> &Equipment);

    UFUNCTION(BlueprintCallable, Category = "ViewFinder")
    void RemoveEquipment(const TScriptInterface<IVFActivatableInterface> &Equipment);
    /**
     * Selects an eligible registered target. An ineligible target leaves the selection unchanged;
     * a failed activation clears it after deactivating the previous equipment.
     */

    UFUNCTION(BlueprintCallable, Category = "ViewFinder")
    bool SwitchEquipment(const TScriptInterface<IVFActivatableInterface> &Equipment);
    /** Advances through the registry, then clears the selection after the last usable entry. */

    UFUNCTION(BlueprintCallable, Category = "ViewFinder")
    void SwitchEquipmentNext();

    UFUNCTION(BlueprintCallable, Category = "ViewFinder")
    void DeactivateCurEquipment();

    UFUNCTION()
    void OnEndRewinding(float Time);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
    int32 EquipmentCurIndex = INDEX_NONE;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
    TArray<TScriptInterface<IVFActivatableInterface>> Equipments;
};
