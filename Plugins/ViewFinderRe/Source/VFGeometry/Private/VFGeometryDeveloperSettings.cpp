// Copyright 2026, StrangeDS. All Rights Reserved.

#include "VFGeometryDeveloperSettings.h"

#include "VFGSNone.h"

UVFGeometryDeveloperSettings::UVFGeometryDeveloperSettings(
    const FObjectInitializer &ObjectInitializer)
{
    GeometryStrategyClass = UVFGSNone::StaticClass();
    ViewFrustumPrimitiveOption = {
        EVF_GeometryScriptPrimitivePolygroupMode::SingleGroup};
    ViewFrustumCollisionOption = {
        false,
        EVF_GeometryScriptCollisionGenerationMethod::ConvexHulls,
        false,
        false,
        false,
        1.0,
        false,
        6,
        1,
        .5f,
        0,
        0.1f,
        0.1f,
        EVF_GeometryScriptSweptHullAxis::Z,
        true,
        0};
}

#if WITH_EDITOR
void UVFGeometryDeveloperSettings::PostEditChangeProperty(
    FPropertyChangedEvent &PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    bool NeedSave = false;

    if (!IsValid(GeometryStrategyClass))
    {
        GeometryStrategyClass = UVFGSNone::StaticClass();
        NeedSave = true;
    }

    if (NeedSave)
    {
        Modify();
        SaveConfig();
    }
}
#endif

bool UVFGeometryDeveloperSettings::IsGeometryStrategyNone() const
{
    if (!IsValid(GeometryStrategyClass))
        return true;

    return GeometryStrategyClass == UVFGSNone::StaticClass();
}

const FVF_GeometryScriptCollisionFromMeshOptions UVFGeometryDeveloperSettings::GetCollisionOption(int Level) const
{
    for (auto &Mapping : CollisionLevels)
    {
        if (Mapping.Contains(Level))
        {
            return Mapping.Option;
        }
    }
    return FVF_GeometryScriptCollisionFromMeshOptions();
}