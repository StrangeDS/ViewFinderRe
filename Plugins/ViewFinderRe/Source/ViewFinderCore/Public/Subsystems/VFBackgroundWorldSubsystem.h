#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VFBackgroundWorldSubsystem.generated.h"

/**
背景的世界子系统, 现仅用于UVFBackgroundCaptureComponent提供ShowOnlyList
 */
UCLASS(ClassGroup = (ViewFinder))
class VIEWFINDERCORE_API UVFBackgroundWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	bool Register(AActor *Background);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder")
	bool Unregister(AActor *Background);

	UFUNCTION(BlueprintPure, Category = "ViewFinder")
	TArray<AActor *> GetBackgrounds();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TArray<AActor *> Backgrounds;
};