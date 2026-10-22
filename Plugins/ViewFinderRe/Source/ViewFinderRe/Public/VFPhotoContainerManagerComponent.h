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
/** Owns an optional photo container without requiring AVFCharacter. */
UCLASS(Blueprintable, ClassGroup=(ViewFinder), meta=(BlueprintSpawnableComponent))
class VIEWFINDERRE_API UVFPhotoContainerManagerComponent : public UActorComponent,
    public IVFPhotoContainerInterface
{
    GENERATED_BODY()
public:
    UVFPhotoContainerManagerComponent();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction) override;
    virtual int GetPhoto2DNum_Implementation() override;
    virtual bool TakeIn_Implementation(AVFPhoto2D* Photo, const bool& Enabled) override;
    /** Tries the pawn, optional manager, then attached actor container interfaces. */
    static bool TryTakeIn(APawn* Pawn, AVFPhoto2D* Photo, UObject** Recipient = nullptr);
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ViewFinder")
    TSubclassOf<AVFPhotoContainer_Input> ContainerClass;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="ViewFinder")
    TObjectPtr<AVFPhotoContainer_Input> Container;
private:
    UFUNCTION()
    void HandleControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);
    UFUNCTION()
    void HandlePhotoNumChanged(int Num);
    UPROPERTY()
    TObjectPtr<UVFEquipmentManagerComponent> EquipmentManager;
    bool bFollowCameraManager = false;
};
