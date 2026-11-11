// CONFIDENCE: HIGH Original addresses: thunk 0x142b3db10; see evidence.json for callees.
// Signature: bool WillVolumeDeleteAfterActorSpawn(AFortVolume* Volume)
if (!Volume)
{
    return false;
}
// Component classes resolved from GetPrivateStaticClassBody names of 0x1429da540, 0x142b86500, 0x14297b320, 0x142b6ac80.
const bool bHasMinigameVolume = Volume->FindComponentByClass(UFortMinigameVolumeComponent::StaticClass()) != nullptr;
const bool bHasPlaysetStream = Volume->FindComponentByClass(UPlaysetLevelStreamComponent::StaticClass()) != nullptr;
const bool bHasFortLevelSave = Volume->FindComponentByClass(UFortLevelSaveComponent::StaticClass()) != nullptr;
const bool bHasLevelSave = Volume->FindComponentByClass(ULevelSaveComponent::StaticClass()) != nullptr;
return !bHasMinigameVolume && !bHasPlaysetStream && !bHasFortLevelSave && bHasLevelSave;
