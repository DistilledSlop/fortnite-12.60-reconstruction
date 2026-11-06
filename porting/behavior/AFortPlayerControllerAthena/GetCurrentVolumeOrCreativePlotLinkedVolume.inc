// CONFIDENCE: high; 0x1419d3790 resolves the manager from this controller, then performs the player-state lookup.
AFortVolumeManager* Manager = AFortVolumeManager::NativeResolveVolumeManager(const_cast<AFortPlayerControllerAthena*>(this));
return Manager ? Manager->NativeFindPlayerVolume(PlayerState) : nullptr;
