// Copyright 2026. All Rights Reserved.
#include "Runtime/Launch/Resources/Version.h"
#if WITH_DEV_AUTOMATION_TESTS && ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "GameFramework/DefaultPawn.h"
#include "GameFramework/PlayerController.h"
#include "VFStepsRecorderWorldSubsystem.h"
#include "VFEquipmentManagerComponent.h"
#include "VFPhotoContainerManagerComponent.h"
#include "VFTimeRewindComponent.h"
#include "VFPhotoContainer_Input.h"
#include "VFPhotoCatcher_PickUp.h"
#include "VFPhoto2D.h"
#include "VFPhoto3D.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "RenderingThread.h"
#include "Misc/ScopeExit.h"
#include "VFDynamicMeshComponent.h"
#include "VFActivatableInterface.h"
#include "VFInteractInterface.h"

namespace VFTests
{
struct FWorld : FTestWorldWrapper
{
    FWorld() { CreateTestWorld(EWorldType::Game); BeginPlayInTestWorld(); }
};
template<class T> T* AddComponent(AActor* Owner)
{
    T* Component = NewObject<T>(Owner);
    Owner->AddInstanceComponent(Component);
    Component->RegisterComponent();
    return Component;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFInvalidSenderTest, "ViewFinderRe.Core.InvalidRewindSender", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FVFInvalidSenderTest::RunTest(const FString& Parameters)
{
    VFTests::FWorld Context;
    auto* Recorder = Context.GetTestWorld()->GetSubsystem<UVFStepsRecorderWorldSubsystem>();
    Recorder->Time = 1.f;
    Recorder->TimeOfStart = 0.05f;
    FVFStepInfo Invalid;
    Invalid.Time = 1.f;
    Invalid.Sender = nullptr;
    Recorder->Infos.Add(Invalid);
    Recorder->TickBackward(0.05f);
    TestEqual(TEXT("Invalid record consumed"), Recorder->Infos.Num(), 0);
    Context.ForwardErrorMessages(this);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFLatePossessionTest, "ViewFinderRe.Components.LatePossessionAndCleanup", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FVFLatePossessionTest::RunTest(const FString& Parameters)
{
    VFTests::FWorld Context;
    UWorld* World = Context.GetTestWorld();
    auto* Pawn = World->SpawnActor<ADefaultPawn>();
    auto* Equipment = VFTests::AddComponent<UVFEquipmentManagerComponent>(Pawn);
    auto* Manager = VFTests::AddComponent<UVFPhotoContainerManagerComponent>(Pawn);
    if (!TestNotNull(TEXT("Container created"), Manager->Container.Get())) return false;
    auto* Container = Manager->Container.Get();
    TestNull(TEXT("Initially unpossessed"), Container->PlayerController.Get());
    auto* Photo = World->SpawnActor<AVFPhoto2D>();
    TestFalse(TEXT("Unpossessed container refuses photo"), UVFPhotoContainerManagerComponent::TryTakeIn(Pawn, Photo));
    auto* Controller = World->SpawnActor<APlayerController>();
    Controller->Possess(Pawn);
    TestEqual(TEXT("Late controller assigned"), Container->PlayerController.Get(), Controller);
    TestTrue(TEXT("Photo accepted after possession"), UVFPhotoContainerManagerComponent::TryTakeIn(Pawn, Photo));
    TestTrue(TEXT("First photo activates container"), Container->IsEnabled());
    Controller->UnPossess();
    TestNull(TEXT("Unpossess clears controller"), Container->PlayerController.Get());
    TestFalse(TEXT("Unpossess deactivates container"), Container->IsEnabled());
    Pawn->Destroy();
    TestFalse(TEXT("Owned container is destroyed"), IsValid(Container));
    Context.ForwardErrorMessages(this);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFCameraFallbackTest, "ViewFinderRe.Components.CameraFallback", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FVFCameraFallbackTest::RunTest(const FString& Parameters)
{
    VFTests::FWorld Context;
    UWorld* World = Context.GetTestWorld();
    auto* Pawn = World->SpawnActor<ADefaultPawn>();
    auto* Controller = World->SpawnActor<APlayerController>();
    Controller->Possess(Pawn);
    auto* Equipment = VFTests::AddComponent<UVFEquipmentManagerComponent>(Pawn);
    auto* Camera = World->SpawnActor<AVFPhotoCatcher_PickUp>();
    TestTrue(TEXT("Generic pawn can pick up camera"), IVFInteractInterface::Execute_Interact(Camera, Controller));
    TestTrue(TEXT("Camera is registered"), Equipment->Equipments.Contains(Camera));
    TestTrue(TEXT("Camera is active"), IVFActivatableInterface::Execute_IsActive(Camera));
    if (TestTrue(TEXT("Valid selected index"), Equipment->Equipments.IsValidIndex(Equipment->EquipmentCurIndex)))
        TestEqual(TEXT("Selected camera"), Equipment->Equipments[Equipment->EquipmentCurIndex].GetObject(), static_cast<UObject*>(Camera));
    TestTrue(TEXT("Camera-manager following tick enabled"), Camera->IsActorTickEnabled());
    Camera->DropDown();
    TestFalse(TEXT("Drop unregisters camera"), Equipment->Equipments.Contains(Camera));
    TestFalse(TEXT("Drop stops following"), Camera->IsActorTickEnabled());
    TestTrue(TEXT("Dropped camera can be interacted with"), IVFInteractInterface::Execute_IsEnabled(Camera, Controller));
    Context.ForwardErrorMessages(this);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFEquipmentRewindTest, "ViewFinderRe.Components.EquipmentRewindAndRemoval", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FVFEquipmentRewindTest::RunTest(const FString& Parameters)
{
    VFTests::FWorld Context;
    UWorld* World = Context.GetTestWorld();
    auto* Pawn = World->SpawnActor<ADefaultPawn>();
    auto* Controller = World->SpawnActor<APlayerController>();
    Controller->Possess(Pawn);
    auto* Manager = VFTests::AddComponent<UVFEquipmentManagerComponent>(Pawn);
    auto* A = World->SpawnActor<AVFPhotoContainer_Input>();
    auto* B = World->SpawnActor<AVFPhotoContainer_Input>();
    for (auto* Container : {A, B})
    {
        Container->SetPlayerController(Controller);
        Container->AddAPhoto(World->SpawnActor<AVFPhoto2D>());
        Manager->AddEquipment(Container);
    }
    TestTrue(TEXT("Select B"), Manager->SwitchEquipment(B));
    B->SetEnabled(false);
    A->SetEnabled(true);
    auto* Recorder = World->GetSubsystem<UVFStepsRecorderWorldSubsystem>();
    Recorder->bIsRewinding = true;
    Recorder->EndRewinding();
    TestEqual(TEXT("End rewind event reconciles active equipment"), Manager->EquipmentCurIndex, 0);
    TestTrue(TEXT("Switch B again"), Manager->SwitchEquipment(B));
    TestFalse(TEXT("A was deactivated"), A->IsEnabled());
    Manager->RemoveEquipment(A);
    TestEqual(TEXT("Removing earlier entry preserves selected index"), Manager->EquipmentCurIndex, 0);
    Manager->DeactivateCurEquipment();
    TestFalse(TEXT("Remaining selected B can be deactivated"), B->IsEnabled());
    Context.ForwardErrorMessages(this);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFEmptyHistoryTest, "ViewFinderRe.Components.EmptyRewindHistory", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FVFEmptyHistoryTest::RunTest(const FString& Parameters)
{
    VFTests::FWorld Context;
    auto* Pawn = Context.GetTestWorld()->SpawnActor<ADefaultPawn>();
    auto* Rewind = VFTests::AddComponent<UVFTimeRewindComponent>(Pawn);
    Rewind->Steps.Reset();
    const FVector Before = Pawn->GetActorLocation();
    Rewind->TickBackward_Implementation(0.f);
    TestEqual(TEXT("Empty history leaves pawn unchanged"), Pawn->GetActorLocation(), Before);
    Context.ForwardErrorMessages(this);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFPreviewDestroyTest, "ViewFinderRe.Components.PreviewDestroyReleasesInput", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FVFPreviewDestroyTest::RunTest(const FString& Parameters)
{
    VFTests::FWorld Context;
    auto* World = Context.GetTestWorld();
    auto* Pawn = World->SpawnActor<ADefaultPawn>();
    auto* PC = World->SpawnActor<APlayerController>();
    PC->Possess(Pawn);
    auto* Camera = World->SpawnActor<AVFPhotoCatcher_PickUp>();
    TestTrue(TEXT("Standalone pickup"), IVFInteractInterface::Execute_Interact(Camera, PC));
    Camera->CloseToPreview();
    TestFalse(TEXT("Preview owns input suppression"), Pawn->InputEnabled());
    Camera->Destroy();
    TestTrue(TEXT("Destroy restores input"), Pawn->InputEnabled());
    Context.ForwardErrorMessages(this);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFHolderLossTest, "ViewFinderRe.Components.HolderLossStopsCamera", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FVFHolderLossTest::RunTest(const FString& Parameters)
{
    VFTests::FWorld Context;
    auto* World = Context.GetTestWorld();
    auto* Pawn = World->SpawnActor<ADefaultPawn>();
    auto* PC = World->SpawnActor<APlayerController>();
    PC->Possess(Pawn);
    auto* Camera = World->SpawnActor<AVFPhotoCatcher_PickUp>();
    TestTrue(TEXT("Pickup"), IVFInteractInterface::Execute_Interact(Camera, PC));
    Camera->CloseToPreview();
    PC->UnPossess();
    TestFalse(TEXT("Unpossess stops following"), Camera->IsActorTickEnabled());
    TestTrue(TEXT("Unpossess returns camera to world"), IVFInteractInterface::Execute_IsEnabled(Camera, PC));
    TestTrue(TEXT("Unpossess restores held preview input"), Pawn->InputEnabled());
    Context.ForwardErrorMessages(this);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFInactiveDropRewindTest, "ViewFinderRe.Components.InactiveDropRewind", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FVFInactiveDropRewindTest::RunTest(const FString& Parameters)
{
    VFTests::FWorld Context;
    auto* World = Context.GetTestWorld();
    auto* Pawn = World->SpawnActor<ADefaultPawn>();
    auto* PC = World->SpawnActor<APlayerController>();
    PC->Possess(Pawn);
    auto* Equipment = VFTests::AddComponent<UVFEquipmentManagerComponent>(Pawn);
    auto* Camera = World->SpawnActor<AVFPhotoCatcher_PickUp>();
    auto* Container = World->SpawnActor<AVFPhotoContainer_Input>();
    Container->SetPlayerController(PC);
    Container->AddAPhoto(World->SpawnActor<AVFPhoto2D>());
    Equipment->AddEquipment(Container);
    IVFInteractInterface::Execute_Interact(Camera, PC);
    Equipment->SwitchEquipment(Container);
    TestFalse(TEXT("Camera inactive before drop"), IVFActivatableInterface::Execute_IsActive(Camera));
    auto* Recorder = World->GetSubsystem<UVFStepsRecorderWorldSubsystem>();
    Recorder->Infos.Reset();
    Recorder->Time = 2.f;
    Camera->DropDown();
    TestEqual(TEXT("Inactive drop adds no spurious deactivation step"), Recorder->Infos.Num(), 1);
    Recorder->bIsRewinding = true;
    Recorder->Time = 1.9f;
    Recorder->TickBackward(0.05f);
    Recorder->EndRewinding();
    TestFalse(TEXT("Undo drop preserves inactive camera"), IVFActivatableInterface::Execute_IsActive(Camera));
    TestTrue(TEXT("Undo inactive drop leaves other equipment active"), Container->IsEnabled());
    TestTrue(TEXT("Undo drop restores held following"), Camera->IsActorTickEnabled());
    Context.ForwardErrorMessages(this);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFPawnMovementRewindTest, "ViewFinderRe.Components.PawnTransformAndVelocityRewind", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FVFPawnMovementRewindTest::RunTest(const FString& Parameters)
{
    VFTests::FWorld Context;
    auto* World = Context.GetTestWorld();
    auto* Pawn = World->SpawnActor<ADefaultPawn>();
    auto* Rewind = VFTests::AddComponent<UVFTimeRewindComponent>(Pawn);
    auto* Recorder = World->GetSubsystem<UVFStepsRecorderWorldSubsystem>();
    Rewind->Steps.Reset();
    Recorder->Time = 1.f;
    Pawn->SetActorLocation(FVector(100, 0, 0));
    Pawn->GetMovementComponent()->Velocity = FVector(10, 0, 0);
    Rewind->TickForward_Implementation(1.f);
    Recorder->Time = 1.05f;
    Pawn->SetActorLocation(FVector(110, 0, 0));
    Pawn->GetMovementComponent()->Velocity = FVector(20, 0, 0);
    Rewind->TickForward_Implementation(1.05f);
    // Exercise dispatch from the world recorder to the registered component.
    Recorder->Tick(0.f);
    Recorder->bIsRewinding = true;
    Recorder->Time = 1.f;
    Recorder->TickBackward(0.05f);
    TestTrue(TEXT("Position restored"), Pawn->GetActorLocation().Equals(FVector(100,0,0), 0.01f));
    TestTrue(TEXT("Non-character Pawn velocity restored"), Pawn->GetMovementComponent()->Velocity.Equals(FVector(10,0,0), 0.01f));
    Recorder->EndRewinding();
    Context.ForwardErrorMessages(this);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFPhotoCaptureFlowTest, "ViewFinderRe.Integration.CapturePlaceWithoutManager", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::NonNullRHI)
bool FVFPhotoCaptureFlowTest::RunTest(const FString& Parameters)
{
    VFTests::FWorld Context;
    UWorld* World = Context.GetTestWorld();
    auto* Pawn = World->SpawnActor<ADefaultPawn>();
    auto* PC = World->SpawnActor<APlayerController>();
    PC->Possess(Pawn);
    auto* Container = World->SpawnActor<AVFPhotoContainer_Input>();
    Container->SetPlayerController(PC);
    Container->AttachToActor(Pawn, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    auto* Camera = World->SpawnActor<AVFPhotoCatcher_PickUp>();
    auto* Cube = World->SpawnActor<AStaticMeshActor>();
    Cube->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (!TestNotNull(TEXT("Test cube asset exists"), CubeMesh)) return false;
    const bool OriginalCPUAccess = CubeMesh->bAllowCPUAccess;
    // The plugin requires CPU-accessible meshes. Restore this in-memory fixture setting.
    CubeMesh->bAllowCPUAccess = true;
    ON_SCOPE_EXIT { CubeMesh->bAllowCPUAccess = OriginalCPUAccess; };
    Cube->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
    Cube->SetActorLocation(FVector(400,0,0));
    TestTrue(TEXT("Pickup without equipment manager"), IVFInteractInterface::Execute_Interact(Camera, PC));
    Camera->CloseToPreview();
    for (int32 Index=0; Index<300; ++Index) Camera->CloseToPreview_Move();
    Context.TickTestWorld(0.02f);
    FlushRenderingCommands();
    AVFPhoto2D* Photo = Camera->TakeAPhoto();
    if (!TestNotNull(TEXT("Real capture creates photo"), Photo)) return false;
    TestNotNull(TEXT("Capture creates texture"), Photo->Texture2D.Get());
    TestNotNull(TEXT("Capture creates 3D photo"), Photo->GetPhoto3D());
    TArray<AActor*> CapturedActors;
    Photo->GetPhoto3D()->GetAttachedActors(CapturedActors, true, true);
    bool HasCopiedGeometry = false;
    for (AActor* Actor : CapturedActors)
        if (auto* Mesh = Actor->FindComponentByClass<UVFDynamicMeshComponent>())
            HasCopiedGeometry |= Mesh->GetDynamicMesh()->GetTriangleCount() > 0;
    TestTrue(TEXT("Photo contains nonempty copied geometry"), HasCopiedGeometry);
    TestEqual(TEXT("Captured photo enters attached container"), Container->Num(), 1);
    TestTrue(TEXT("Container active"), Container->IsEnabled());
    TestFalse(TEXT("Camera deactivated without manager"), IVFActivatableInterface::Execute_IsActive(Camera));
    TestTrue(TEXT("Preview releases pawn input"), Pawn->InputEnabled());
    Container->PlaceCurrentPhoto();
    TestEqual(TEXT("Place removes photo from container"), Container->Num(), 0);
    TestTrue(TEXT("Normal placed photo remains hidden, no re-pickup extension"), Photo->IsHidden());
    Context.ForwardErrorMessages(this);
    return true;
}
#endif
