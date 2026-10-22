// Copyright 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFPhotoContainerInterface.h"
#include "VFPhotoContainerManagerComponent.generated.h"

class AVFPhotoContainer_Input;
class UVFEquipmentManagerComponent;
class APawn;
class AController;

/**
 * Owns a photo container for a pawn that does not inherit AVFCharacter.
 * Follows its camera component, or the player camera manager when no camera component exists.
 * Controller changes are supported after BeginPlay; EndPlay destroys the owned container.
 */
UCLASS(Blueprintable, ClassGroup = (ViewFinder), meta = (BlueprintSpawnableComponent))
class VIEWFINDERRE_API UVFPhotoContainerManagerComponent : public UActorComponent,
                                                           public IVFPhotoContainerInterface
{
    GENERATED_BODY()

public:
    UVFPhotoContainerManagerComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction *TickFunction) override;
    virtual int GetPhoto2DNum_Implementation() override;
    virtual bool TakeIn_Implementation(AVFPhoto2D *Photo, const bool &Enabled) override;

    /**
     * Tries the pawn interface, its manager component, then directly attached container actors.
     * Returns true when a container accepts Photo. OutRecipient, when supplied, receives that
     * object (the owned container for a manager) or nullptr on failure.
     * A first photo may activate the receiving container. The caller handles its own equipment.
     */
    static bool TryTakeIn(APawn *Pawn, AVFPhoto2D *Photo, UObject **OutRecipient = nullptr);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
    TSubclassOf<AVFPhotoContainer_Input> ContainerClass;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "ViewFinder")
    TObjectPtr<AVFPhotoContainer_Input> Container;

private:
    UFUNCTION()
    void HandleControllerChanged(APawn *Pawn, AController *OldController, AController *NewController);

    void ActivateContainerForFirstPhoto();
    static bool TryTakeInAttachedContainer(AActor *Actor, AVFPhoto2D *Photo);

    UPROPERTY()
    TObjectPtr<UVFEquipmentManagerComponent> EquipmentManager;

    bool bFollowCameraManager = false;
};
