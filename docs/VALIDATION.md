# Validation

Validated locally on macOS with UE 5.7, Xcode 27.1, Mac SDK 27.0. UE 5.6 did not build in this environment: its configured Xcode maximum is 27.0.0. Windows and UE 5.3/5.6 are not claimed as tested.

The tests in `VFGenericIntegrationTests.cpp` use UE's FTestWorldWrapper (guarded for UE 5.6+). They run real Actors, components, controller possession, the actual rewind subsystem, and a Metal-rendered capture/place flow. They do not substitute mocks for these responsibilities.

Nine logic/lifecycle tests passed together. The separate `ViewFinderRe.Integration.CapturePlaceWithoutManager` test then passed after configuring the fixture mesh for Allow CPU Access as required by the plugin. It verifies a generated texture, a 3D photo containing nonempty copied geometry, attached-container insertion and activation, camera deactivation, input restoration, and normal placement. The capture test reports one pre-existing CaptureScene/bCaptureEveryFrame efficiency warning and zero errors. This is not a visual-quality evaluation or a packaged-game test.

Earlier failed runs were retained locally: 7/8 initially, exposing an UnPossess input-restore bug; then 9/10, with the integration fixture rejected for missing CPU access. The production bug was fixed; the fixture now temporarily enables CPU access in memory and restores the original value on scope exit.

For this Mac validation only, an ignored standalone harness adds Mac to platform allowlists, selects UE 5.7 build settings, and supplies the non-Windows __FUNCTIONW__ log shim. These adjustments are not part of the upstream plugin descriptor/patch and do not claim official Mac support. All other plugin source files were byte-compared with the clean contribution source before delivery.

Run the Editor automation prefix `ViewFinderRe` for logic tests, and `ViewFinderRe.Integration.CapturePlaceWithoutManager` with a non-null RHI for capture/placement. Use a CPU-accessible test mesh. The local Delivery folder contains the exact preparation script, commands, raw logs and JSON results.

Independent review: GPT-6.1-Sol reviewed the changes and rechecked the fixes. Its final review was limited to the changed lifecycle, rewind and integration paths, not a proof of general correctness. CameraManager fallback keeps upstream post-process behavior: filters specifically requiring a CameraComponent are not extended by this patch.

## Readability pass

The generic integration code now uses consistent four-space Allman formatting, explicit branch braces, and grouped declarations. Container lookup, first-photo activation, camera-holder binding, view tracking and equipment registration have separate names and responsibilities. The duplicated pawn-velocity restore path is shared. Public comments describe inputs, outputs and ownership; implementation comments explain lifecycle and rewind decisions. No new fallback layer or defensive error-handling framework was added. UObject and Blueprint event boundaries retain their lifetime checks.

The public interfaces and intended behavior are unchanged. The refactor was independently checked with GPT-6.1-Sol, rebuilt on UE 5.7 and rerun as one full Metal automation suite: all 10 tests passed, including the capture/place integration test (one existing performance warning).
