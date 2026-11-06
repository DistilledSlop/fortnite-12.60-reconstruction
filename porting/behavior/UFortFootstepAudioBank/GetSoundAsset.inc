// CONFIDENCE: HIGH
// Native 0x1424aea20: position selects a 3P bank; 1P always uses SoundAssets.
const uint8 Surface = static_cast<uint8>(SurfaceType);
const uint8 Step = static_cast<uint8>(StepType);
if (bIsLocal)
{
    return SurfaceData1P[Surface].SoundAssets[Step];
}
const FFortFootstepSurfaceAudioData& Data = SurfaceData3P[Surface];
if (StepPosition == EFortFootstepPosition::Above)
{
    return Data.SoundAssetsAbove[Step];
}
if (StepPosition == EFortFootstepPosition::Below)
{
    return Data.SoundAssetsBelow[Step];
}
return Data.SoundAssets[Step];
