#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"

#include "VFPawnStandIn.generated.h"

UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API AVFPawnStandIn : public AActor
{
	GENERATED_BODY()
	
public:	
	AVFPawnStandIn();

public:	
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void BeginDestroy() override;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void SetTargetPawn(APawn *Pawn);
	
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	void TeleportTargetPawn();
	
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	void Hide();
	
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ViewFinder")
	FORCEINLINE UVFDynamicMeshComponent *GetVFDMComp() { return DynamicMesh; };
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class APawn> TargetPawn;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UStaticMeshComponent> StaticMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UVFDynamicMeshComponent> DynamicMesh;
	
	UPROPERTY()
	TObjectPtr<class UStaticMesh> StaticMeshObject;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<class UVFHelperComponent> Helper;
};
