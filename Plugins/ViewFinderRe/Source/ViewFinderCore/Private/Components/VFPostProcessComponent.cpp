#include "VFPostProcessComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Materials/MaterialInterface.h"

#include "VFCommon.h"

UVFPostProcessComponent::UVFPostProcessComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

#if WITH_EDITOR
#include "MaterialDomain.h"

void UVFPostProcessComponent::BeginPlay()
{
	Super::BeginPlay();

	if (IsAnyRule() && !IsValid(PostProcess))
	{
		VF_LOG(Error, TEXT("%s %s has invalid PoseProcess."),
			   *GetOwner()->GetActorLabel(), __FUNCTIONW__);
	}
	if (IsValid(PostProcess) &&
		PostProcess->GetMaterial()->MaterialDomain != EMaterialDomain::MD_PostProcess)
	{
		VF_LOG(Error, TEXT("%s PoseProcess Domain is not MD_PostProcess."),
			   *GetOwner()->GetActorLabel());
	}
}
#endif

bool UVFPostProcessComponent::IsAnyRule()
{
	return Rule != EVFStencilRule::None;
}

void UVFPostProcessComponent::AddOrUpdateSceneCapturePostProcess(USceneCaptureComponent2D *SceneCapture)
{
	check(IsValid(SceneCapture));
	ensureMsgf(IsValid(PostProcess), TEXT("%s invalid PostProcess."));
	SceneCapture->AddOrUpdateBlendable(PostProcess);
}

void UVFPostProcessComponent::RemoveSceneCapturePostProcess(USceneCaptureComponent2D *SceneCapture)
{
	check(IsValid(SceneCapture));
	// if (ensureMsgf(IsValid(PostProcess), TEXT("%s invalid PostProcess.")))
	SceneCapture->RemoveBlendable(PostProcess);
}

void UVFPostProcessComponent::AddOrUpdateCameraPostProcess(UCameraComponent *Camera)
{
	check(IsValid(Camera));
	ensureMsgf(IsValid(PostProcess), TEXT("%s invalid PostProcess."));
	Camera->AddOrUpdateBlendable(PostProcess);
}

void UVFPostProcessComponent::RemoveCameraPostProcess(UCameraComponent *Camera)
{
	check(IsValid(Camera));
	// if (ensureMsgf(IsValid(PostProcess), TEXT("%s invalid PostProcess.")))
	Camera->RemoveBlendable(PostProcess);
}

void UVFPostProcessComponent::SetStencilValueNext_Implementation(UPrimitiveComponent *Comp)
{
	check(Comp);

	switch (Rule)
	{
	case EVFStencilRule::None:
		break;
	case EVFStencilRule::Original:
	{
		Comp->bRenderCustomDepth = true;
		break;
	}
	case EVFStencilRule::Fixed:
	{
		Comp->bRenderCustomDepth = true;
		Comp->CustomDepthStencilValue = StencilBase;
		break;
	}
	case EVFStencilRule::Growing:
	{
		Comp->bRenderCustomDepth = true;
		Comp->CustomDepthStencilValue += StencilBase;
		break;
	}
	default:
	{
		VF_LOG(Warning, TEXT("%s doesn't handle Rule(%i).") __FUNCTIONW__, Rule);
		break;
	}
	}
}

int UVFPostProcessComponent::GetStencilValueNext_Implementation(int Stencil)
{
	switch (Rule)
	{
	case EVFStencilRule::None:
	case EVFStencilRule::Original:
	{
		return Stencil;
	}
	case EVFStencilRule::Fixed:
	{
		return StencilBase;
	}
	case EVFStencilRule::Growing:
	{
		return Stencil + StencilBase;
	}
	default:
	{
		VF_LOG(Warning, TEXT("%s doesn't handle Rule(%i).") __FUNCTIONW__, Rule);
		return Stencil;
	}
	}
}