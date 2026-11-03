// CONFIDENCE: high; 0x1419dd9e0 calls the volume type helper twice before comparing either result.
AFortVolumeManager* Manager = AFortVolumeManager::NativeResolveVolumeManager(const_cast<AFortPlayerControllerAthena*>(this));
AFortVolume* Volume = Manager ? Manager->NativeFindPlayerVolume(PlayerState) : nullptr;
if (!Volume)
{
    return false;
}
const uint8 First = NativeGetVolumeType96(Volume);
const uint8 Second = NativeGetVolumeType96(Volume);
return First == 3 || Second == 2;
