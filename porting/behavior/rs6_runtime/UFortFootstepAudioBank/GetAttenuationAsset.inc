// Behaviour snippet: UFortFootstepAudioBank::GetAttenuationAsset (body only; spliced into the generated member function).
// Original address: 0x1424a9610
// Offset evidence: AttenuationData1P[7] at +0xd80 and AttenuationData3P[7] at +0xe98, 40-byte FFortFootstepAttenuationData elements indexed by StepType. Local path reads SoundAttenuation (+0). Non-local path reads field +0x8 (SoundAttenuationAbove) for position 1, +0x10 (SoundAttenuationBelow) for 2, +0x18 (SoundAttenuationAboveOrBelowAndVisible) for 3, and +0x0 (SoundAttenuation) otherwise.
// CONFIDENCE: HIGH
const int32 TypeIndex = static_cast<int32>(StepType);
if (bIsLocal)
{
	return AttenuationData1P[TypeIndex].SoundAttenuation;
}
const int32 PositionIndex = static_cast<int32>(StepPosition);
if (PositionIndex == 1)
{
	return AttenuationData3P[TypeIndex].SoundAttenuationAbove;
}
if (PositionIndex == 2)
{
	return AttenuationData3P[TypeIndex].SoundAttenuationBelow;
}
if (PositionIndex == 3)
{
	return AttenuationData3P[TypeIndex].SoundAttenuationAboveOrBelowAndVisible;
}
return AttenuationData3P[TypeIndex].SoundAttenuation;
