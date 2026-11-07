// Behaviour snippet: UFortUICameraManagerComponent::GetCameraActor (body only; spliced into the generated member function).
// Original body: 0x141ca62f0 (thunk 0x142b29250). Offset evidence: CurrentCamera at +0xba (Enum, one byte) is the key of CameraMapping (TMap<EFrontEndCamera, AFortCameraBase*> at +0xc0). The asm tests Elements.Num (+0xc8) against NumFreeIndices (+0xf4) for the empty case, buckets with (HashSize at +0x108 - 1) & key, walks the chain, and returns the value at element+0x8. That is TMap::Num() == 0, then TMap::FindRef(key).
// CONFIDENCE: MEDIUM
if (CameraMapping.Num() == 0)
{
    return nullptr;
}
return CameraMapping.FindRef(CurrentCamera);
