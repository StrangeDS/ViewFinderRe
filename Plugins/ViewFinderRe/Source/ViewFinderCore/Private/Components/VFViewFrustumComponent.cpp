#include "VFViewFrustumComponent.h"

#include "UObject/ConstructorHelpers.h"

UVFViewFrustumComponent::UVFViewFrustumComponent()
{
    CastShadow = false;
    SetComplexAsSimpleCollisionEnabled(false);
    // 对比OverlapAll差异为: ViewFrustum物体与ViewFrustum物体为ignore
    SetCollisionProfileName(TEXT("ViewFrustum"));

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialSelector(
        TEXT("/ViewFinderRe/Materials/Frustum/MI_Transparent.MI_Transparent"));
    Matirial = MaterialSelector.Object;
    SetMaterial(0, Matirial);
}

void UVFViewFrustumComponent::BeginPlay()
{
    Super::BeginPlay();

    SetHiddenInGame(true);
}

void UVFViewFrustumComponent::GenerateViewFrustum_Implementation(float Angle, float AspectRatio, float StartDis, float EndDis)
{
    check(Angle > 0.0f && Angle < 360.0f);
    check(AspectRatio > 0.0f);
    check(StartDis > 0.0f);
    check(EndDis > 0.0f && StartDis < EndDis);

    UVFGeometryFunctions::AppendFrustum(MeshObject, PrimitiveOptions, Angle, AspectRatio, StartDis, EndDis);
    UVFGeometryFunctions::SetDynamicMeshCollisionFromMesh(MeshObject, this, CollisionOptions);
}

void UVFViewFrustumComponent::RegenerateViewFrustum(float Angle, float AspectRatio, float StartDis, float EndDis)
{
    MeshObject->Reset();
    GenerateViewFrustum(Angle, AspectRatio, StartDis, EndDis);
}

void UVFViewFrustumComponent::RecordViewFrustum(UVFViewFrustumComponent *Other)
{
    UVFGeometryFunctions::CopyMeshFromComponent(
        Other,
        MeshObject,
        FVF_GeometryScriptCopyMeshFromComponentOptions(),
        false);
    UVFGeometryFunctions::SetDynamicMeshCollisionFromMesh(MeshObject, this, CollisionOptions);
}

#if WITH_EDITOR
#include "VFCommon.h"

void UVFViewFrustumComponent::DrawConvexCollision()
{
    auto KAggregateGeom = GetSimpleCollisionShapes();
    auto ConvexElems = KAggregateGeom.ConvexElems;
    FTransform Transform = GetComponentTransform();

    VF_LOG(Log, TEXT("ConvexElems.Num(): %i"), ConvexElems.Num());
    for (auto Elem : ConvexElems)
    {
        auto VertexData = Elem.VertexData;
        VF_LOG(Log, TEXT("VertexData.Num(): %i"), VertexData.Num());
        for (int i = 0; i < VertexData.Num(); ++i)
        {
            VertexData[i] = Transform.TransformPosition(VertexData[i]);
        }
        DrawDebugMesh(
            GetWorld(),
            VertexData,
            Elem.IndexData,
            FColor(0, 100, 0, 100),
            false, 5.0f, SDPG_Foreground);
    }
}
#endif