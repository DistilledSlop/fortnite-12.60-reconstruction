// CONFIDENCE: HIGH
// Native 0x1424ab270: 1P volume bypasses the teammate multiplier.
const uint8 Index = static_cast<uint8>(StepType);
if (bIsLocal)
{
    return AttenuationData1P[Index].VolumeMultiplier;
}
const float Volume = AttenuationData3P[Index].VolumeMultiplier;
return bApplyTeammateVolume ? Volume * TeammateVolumeMultiplier : Volume;
