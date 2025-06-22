#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFStandInInterface.h"
#include "VFHelperInterface.h"
#include "VFPawnStandIn.generated.h"

class APawn;
class UStaticMesh;
class UStaticMeshComponent;

class UVFHelperComponent;
class UVFDynamicMeshComponent;

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPawnStandIn : public AActor,
										  public IVFStandInInterface,
										  public IVFHelperInterface
{
	GENERATED_BODY()

public:
	AVFPawnStandIn();

public:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	FQuat ViewQuat = FQuat::Identity;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void SetTargetPawn(APawn *Pawn);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	void TeleportTargetPawn(UObject *Sender);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void Hide(UObject *Sender);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<APawn> TargetPawn;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UVFDynamicMeshComponent> DynamicMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> StaticMeshObject;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder",
			  meta = (NoEditInline))
	TObjectPtr<UVFHelperComponent> Helper;

public: // implements IVFStandInInterface
	virtual void SetOriginalActor_Implementation(AActor *Source) override;

	virtual UPrimitiveComponent *GetPrimitiveComp_Implementation() override;

public: // IVFHelperInterface
	virtual UVFHelperComponent *GetHelper_Implementation() override;
};
