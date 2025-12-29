// Copyright 2026, StrangeDS. All Rights Reserved.

#include "VFPCommonFunctions.h"

#include "Engine/World.h"

#include "VFDynamicMeshComponent.h"
#include "VFStandInInterface.h"

AActor *UVFPCommonFunctions::ReplaceWithStandIn(AActor *OriginalActor, TSubclassOf<AActor> StandInActorClass)
{
	auto Transform = OriginalActor->GetActorTransform();
	auto StandIn = OriginalActor->GetWorld()->SpawnActor<AActor>(
		StandInActorClass,
		Transform);
	IVFStandInInterface::Execute_SetOriginalActor(StandIn, OriginalActor);
	return StandIn;
}

void UVFPCommonFunctions::K2_GetCompsToHelpersMapping(UPARAM(ref) TArray<UPrimitiveComponent *> &Components, UPARAM(ref) TMap<UPrimitiveComponent *, UVFHelperComponent *> &Map)
{
	GetCompsToHelpersMapping<UPrimitiveComponent>(Components, Map);
}

FTransform UVFPCommonFunctions::TransformLerp(const FTransform &Original, const FTransform &Target, float delta)
{
	FRotator Rot = FMath::Lerp(Original.Rotator(), Target.Rotator(), delta);
	FVector Loc = FMath::Lerp(Original.GetLocation(), Target.GetLocation(), delta);
	return FTransform(Rot, Loc, Original.GetScale3D());
}

FTransform UVFPCommonFunctions::TransformLerpNoScale(const FTransform &Original, const FTransform &Target, float delta)
{
	FRotator Rot = FMath::Lerp(Original.Rotator(), Target.Rotator(), delta);
	FVector Loc = FMath::Lerp(Original.GetLocation(), Target.GetLocation(), delta);
	return FTransform(Rot, Loc, Original.GetScale3D());
}

bool UVFPCommonFunctions::IsEditorCreated(UObject *Object)
{
	return Object->HasAnyFlags(RF_WasLoaded | RF_LoadCompleted);
}