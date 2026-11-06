// CONFIDENCE: high; 0x1419dd990 uses the volume type helper once and tests value 3.
AFortVolumeManager* Manager = AFortVolumeManager::NativeResolveVolumeManager(const_cast<AFortPlayerControllerAthena*>(this));
AFortVolume* Volume = Manager ? Manager->NativeFindPlayerVolume(PlayerState) : nullptr;
return Volume && NativeGetVolumeType96(Volume) == 3;
