#include "VFBackgroundCaptureComponent.h"

#include "Components/StaticMeshComponent.h"

#include "VFPlaneActor.h"
#include "Subsystems/VFBackgroundWorldSubsystem.h"

UVFBackgroundCaptureComponent::UVFBackgroundCaptureComponent()
    : Super()
{
    PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
    PlaneActorClass = AVFPlaneActor::StaticClass();
}

UPrimitiveComponent *UVFBackgroundCaptureComponent::DrawABackgroundWithSize(
    float Distance, float Width, float Height)
{
    check(PlaneActorClass);

    auto PlaneActor = GetWorld()->SpawnActor<AVFPlaneActor>(PlaneActorClass);
    FVector BasePosition = GetComponentLocation();
    FVector RelativePosition = Distance * GetComponentRotation().RotateVector(FVector::ForwardVector);
    FVector Direction = RelativePosition.GetSafeNormal();

#if WITH_EDITOR
    if (GetWorld() && GetWorld()->WorldType != EWorldType::Editor)
    {
#endif
        if (auto BackgroundSystem = GetWorld()->GetSubsystem<UVFBackgroundWorldSubsystem>())
        {
            ShowOnlyActors = BackgroundSystem->GetBackgrounds();
        }
#if WITH_EDITOR
    }
#endif

    // 需要开启后才会有后处理效果.
    bAlwaysPersistRenderingState = true;
    CaptureScene();
    bAlwaysPersistRenderingState = false;
    PlaneActor->SetPlane(RelativePosition + BasePosition, Direction, Width, Height);
    PlaneActor->SetPlaneMaterial(DrawATexture2D());

    return PlaneActor->Plane;
}

UPrimitiveComponent *UVFBackgroundCaptureComponent::DrawABackground()
{
    float Distance = MaxViewDistanceOverride * DistanceRate;
    float FOVRad = FMath::DegreesToRadians(FOVAngle / 2);
    float Width = Distance * FMath::Tan(FOVRad) * 2;
    float Height = Width / AspectRatio;

    return DrawABackgroundWithSize(Distance, Width, Height);
}