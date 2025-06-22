#include "VFPhotoCaptureComponent.h"

#include "TextureResource.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "VFCommon.h"

UVFPhotoCaptureComponent::UVFPhotoCaptureComponent()
{
	bCaptureEveryFrame = false;
	bCaptureOnMovement = false;
	bAlwaysPersistRenderingState = false;
	PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
}

void UVFPhotoCaptureComponent::BeginPlay()
{
	Super::BeginPlay();

	Init();
}

void UVFPhotoCaptureComponent::Init()
{
	if (!IsValid(TextureTarget))
	{
		TextureTarget = NewObject<UTextureRenderTarget2D>(this);
		TextureTarget->InitCustomFormat(TargetWidth, TargetHeight, PF_FloatRGBA, false);
		TextureTarget->ClearColor = FLinearColor::Black;
	}

	auto Actor = GetOwner();
	while (Actor)
	{
		HiddenActors.AddUnique(Actor);
		Actor = Actor->GetParentActor();
	}
}

void UVFPhotoCaptureComponent::ResizeTarget(int Width, int Height)
{
	check(Width > 0 && Height > 0);

	TargetWidth = Width;
	TargetHeight = Height;
	TextureTarget->ResizeTarget(TargetWidth, TargetHeight);
}

void UVFPhotoCaptureComponent::StartDraw()
{
	bCaptureEveryFrame = true;
}

void UVFPhotoCaptureComponent::EndDraw()
{
	bCaptureEveryFrame = false;
}

UTexture2D *UVFPhotoCaptureComponent::DrawATexture2D()
{
	UTexture2D *Texture = nullptr;
	FIntPoint Size(TextureTarget->SizeX, TextureTarget->SizeY);
	FTextureRenderTargetResource *RTResource = TextureTarget->GameThread_GetRenderTargetResource();
	const auto Format = TextureTarget->GetFormat();

	if (Format == EPixelFormat::PF_R8G8B8A8)
	{
		TArray<FColor> PixelData;
		RTResource->ReadPixels(PixelData);

		Texture = UTexture2D::CreateTransient(Size.X, Size.Y, PF_R8G8B8A8);
		FTexture2DMipMap &Mip = Texture->GetPlatformData()->Mips[0];
		void *Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
		FMemory::Memcpy(Data, PixelData.GetData(), PixelData.Num() * sizeof(FColor));
		Mip.BulkData.Unlock();
#if WITH_EDITOR
		Texture->Source.Init2DWithMipChain(
			Texture->GetSizeX(),
			Texture->GetSizeY(),
			ETextureSourceFormat::TSF_BGRA8);

		uint8 *MipData = Texture->Source.LockMip(0);
		FMemory::Memcpy(MipData, PixelData.GetData(), PixelData.Num() * sizeof(FColor));
		Texture->Source.UnlockMip(0);
#endif
	}
	else if (Format == EPixelFormat::PF_FloatRGBA)
	{
		TArray<FFloat16Color> PixelData;
		RTResource->ReadFloat16Pixels(PixelData);

		Texture = UTexture2D::CreateTransient(Size.X, Size.Y, PF_FloatRGBA);
		Texture->CompressionSettings = TC_HDR;
		FTexture2DMipMap &Mip = Texture->GetPlatformData()->Mips[0];
		void *Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
		FMemory::Memcpy(Data, PixelData.GetData(), PixelData.Num() * sizeof(FFloat16Color));
		Mip.BulkData.Unlock();
#if WITH_EDITOR
		Texture->Source.Init2DWithMipChain(
			Texture->GetSizeX(),
			Texture->GetSizeY(),
			ETextureSourceFormat::TSF_RGBA16F);

		uint8 *MipData = Texture->Source.LockMip(0);
		FMemory::Memcpy(MipData, PixelData.GetData(), PixelData.Num() * sizeof(FFloat16Color));
		Texture->Source.UnlockMip(0);
#endif
	}

	if (!ensure(IsValid(Texture)))
	{
		VF_LOG(Error, TEXT("Unimplemented EPixelFormat."));
		return nullptr;
	}

	Texture->UpdateResource();
	return Texture;
}