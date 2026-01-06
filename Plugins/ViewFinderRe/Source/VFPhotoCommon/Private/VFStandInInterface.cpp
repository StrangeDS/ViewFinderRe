// Copyright 2026, StrangeDS. All Rights Reserved.

#include "VFStandInInterface.h"

#include "GameFramework/Actor.h"

#include "VFHelperComponent.h"

void IVFStandInInterface::SetOriginalActor_Implementation(AActor *Original)
{
    auto Actor = Cast<AActor>(_getUObject());
    auto Helper = Actor->GetComponentByClass<UVFHelperComponent>();
    Helper->ActorStandInFor = Original;
}

AActor *IVFStandInInterface::GetOriginalActor_Implementation()
{
    auto Actor = Cast<AActor>(_getUObject());
    auto Helper = Actor->GetComponentByClass<UVFHelperComponent>();
    return Helper->ActorStandInFor;
}

UPrimitiveComponent *IVFStandInInterface::GetPrimitiveComp_Implementation()
{
    unimplemented();
    return nullptr;
}