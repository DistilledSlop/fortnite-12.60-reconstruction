// CONFIDENCE: high; 0x1419dda50 looks up the player volume and tests state byte 1 or bit +0x2d8 mask 1.
AFortVolumeManager* Manager = AFortVolumeManager::NativeResolveVolumeManager(const_cast<AFortPlayerControllerAthena*>(this));
AFortVolume* Volume = Manager ? Manager->NativeFindPlayerVolume(PlayerState) : nullptr;
return Volume && (Volume->VolumeState == EVolumeState::ReadOnly || Volume->bNeverAllowSaving);
