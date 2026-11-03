// CONFIDENCE: HIGH
AFortVolumeManager* Manager = AFortVolumeManager::NativeResolveVolumeManager(this);
if (Manager)
{
    AFortVolume* Volume = Manager->NativeFindPlayerVolume(this);
    if (Volume)
    {
        AFortVolume::NativeAppendMinigameOwner(Volume, OutOwners);
        OutOwners.AddUnique(Volume);
    }
}
