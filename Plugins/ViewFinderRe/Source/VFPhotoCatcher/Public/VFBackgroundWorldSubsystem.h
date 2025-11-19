// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VFBackgroundWorldSubsystem.generated.h"

class UVFUObjsRegistarWorldSubsystem;

/*
Background world subsystem,
currently only used by UVFBackgroundCaptureComponent to provide ShowOnlyList.
*/
UCLASS(ClassGroup = (ViewFinder))
class VFPHOTOCATCHER_API UVFBackgroundWorldSubsystem : public UWorldSubsystem
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
			  Category = "ViewFinder")
	FString ChannelName = TEXT("Background");
};