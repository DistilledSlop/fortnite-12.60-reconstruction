// CONFIDENCE: HIGH -- original helper 0x141edde40; render command 0x141ec7b10.
{
	if (FogMask == nullptr || FogMask->Resource == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UpdateMapTexture() failed.  FogMask or FogMask->Resource is null."));
		return;
	}

	if (!MapData.IsValidIndex(NativeCurrentTeamMapIndex_0x4b8))
	{
		UE_LOG(LogTemp, Error, TEXT("UpdateMapTexture() failed.  Unable to find local map data."));
		return;
	}

	uint8* const Pixels = MapData[NativeCurrentTeamMapIndex_0x4b8].NativeFogPixels_0x20.GetData();
	if (Pixels == nullptr)
	{
		return;
	}

	FTexture2DResource* const TextureResource = static_cast<FTexture2DResource*>(FogMask->Resource);
	const FUpdateTextureRegion2D Region(0, 0, 0, 0, 256, 256);
	ENQUEUE_RENDER_COMMAND(UpdateMapTexture)(
		[TextureResource, Region, Pixels](FRHICommandListImmediate& RHICmdList)
		{
			const int32 CurrentFirstMip = TextureResource->GetCurrentFirstMip();
			if (0 >= CurrentFirstMip)
			{
				RHIUpdateTexture2D(TextureResource->GetTexture2DRHI(), -CurrentFirstMip,
					Region, 256, Pixels + Region.SrcY * 256 + Region.SrcX);
			}
		});
	NativeMapState_0x4c0 &= ~uint32(2);
}
