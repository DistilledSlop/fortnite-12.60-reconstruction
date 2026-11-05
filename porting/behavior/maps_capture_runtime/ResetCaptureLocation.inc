// CONFIDENCE: HIGH -- original exec 0x14294cc60.
const AFortWorldSettings* FortWorldSettings = Cast<AFortWorldSettings>(GetWorldSettings());
if (FortWorldSettings && FortWorldSettings->bOverrideMainMapSettings)
{
    NativeCaptureLocation_0x338 = FVector(FortWorldSettings->PvPMapWorldCenter.X, FortWorldSettings->PvPMapWorldCenter.Y, FortWorldSettings->MapZOffset);
}
else
{
    NativeCaptureLocation_0x338 = FVector(0.0f, 0.0f, OffsetZ);
}
