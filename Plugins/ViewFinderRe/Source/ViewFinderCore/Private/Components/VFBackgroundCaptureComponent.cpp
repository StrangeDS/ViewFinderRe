#include "VFBackgroundCaptureComponent.h"

#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"

#include "VFPlaneActor.h"
#include "Subsystems/VFBackgroundWorldSubsystem.h"

UVFBackgroundCaptureComponent::UVFBackgroundCaptureComponent()
    : Super()
{
    PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
    PlaneActorClass = AVFPlaneActor::StaticClass();
}

#if WITH_EDITOR
#include "Misc/DataValidation.h"

EDataValidationResult UVFBackgroundCaptureComponent::IsDataValid(FDataValidationContext &Context) const
{
	if (Super::IsDataValid(Context) == EDataValidationResult::Invalid)
		return EDataValidationResult::Invalid;

    if (!PlaneActorClass.Get())
    {
        Context.AddError(FText::FromString(TEXT("PlaneActorClass is invalid.")));
        return EDataValidationResult::Invalid;
    }

    return EDataValidationResult::Valid;
}
#endif

UPrimitiveComponent *UVFBackgroundCaptureComponent::DrawABackgroundWithSize(
    float Distance, float Width, float Height)
{
    check(PlaneActorClass);

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

    auto PlaneActor = GetWorld()->SpawnActor<AVFPlaneActor>(PlaneActorClass);
    FVector BasePosition = GetComponentLocation();
    FVector RelativePosition = Distance * GetComponentRotation().RotateVector(FVector::ForwardVector);
    FVector Direction = RelativePosition.GetSafeNormal();

    PlaneActor->SetPlane(RelativePosition + BasePosition, Direction, Width, Height);
    PlaneActor->SetPlaneMaterial(DrawATexture2D());

#if WITH_EDITOR
    if (GetWorld() && GetWorld()->WorldType != EWorldType::Editor)
    {
#endif
        ShowOnlyActors.Reset();
#if WITH_EDITOR
    }
#endif

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