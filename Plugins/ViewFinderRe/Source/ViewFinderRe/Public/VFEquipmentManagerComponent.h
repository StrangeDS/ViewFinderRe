// Copyright 2026. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFActivatableInterface.h"
#include "VFEquipmentManagerComponent.generated.h"

/** Optional equipment management for pawns that do not inherit AVFCharacter. */
UCLASS(Blueprintable, ClassGroup=(ViewFinder), meta=(BlueprintSpawnableComponent))
class VIEWFINDERRE_API UVFEquipmentManagerComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UVFEquipmentManagerComponent();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    UFUNCTION(BlueprintCallable, Category="ViewFinder")
    void AddEquipment(const TScriptInterface<IVFActivatableInterface>& Equipment);
    UFUNCTION(BlueprintCallable, Category="ViewFinder")
    void RemoveEquipment(const TScriptInterface<IVFActivatableInterface>& Equipment);
    UFUNCTION(BlueprintCallable, Category="ViewFinder")
    bool SwitchEquipment(const TScriptInterface<IVFActivatableInterface>& Equipment);
    UFUNCTION(BlueprintCallable, Category="ViewFinder")
    void SwitchEquipmentNext();
    UFUNCTION(BlueprintCallable, Category="ViewFinder")
    void DeactivateCurEquipment();
    UFUNCTION()
    void OnEndRewinding(float Time);
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ViewFinder")
    int32 EquipmentCurIndex = INDEX_NONE;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ViewFinder")
    TArray<TScriptInterface<IVFActivatableInterface>> Equipments;
};
