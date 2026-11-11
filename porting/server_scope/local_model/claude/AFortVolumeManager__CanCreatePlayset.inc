// CONFIDENCE: LOW. Decides whether Thrower may place NewPlayset at StartLocation with PlaysetRotation. All inputs and the volume manager must be present. The thrower's controller must allow creation in its current volume, the thrower's volume must be the volume at the start location, and the playset must fit the volume bounds. Logs the reason for each refusal.
// Original addresses: exec 0x142b3b0c0 (thunk; calls 0x141c67270 with the thrower, start location, rotation and playset). Helpers: 0x141c74820 (volume manager, context argument dropped by the decompile), 0x141c7cde0 (location volume), 0x1419c3880 (NativeCanPlayerCreateInCurrentVolume, out volume), 0x141c94dc0 (NativeCanSpawnVolume).

AFortVolumeManager* VolumeManager = GetVolumeManagerForActor(Thrower);
if (VolumeManager == nullptr || Thrower == nullptr || NewPlayset == nullptr)
{
    UE_LOG(LogFortVolume, Verbose, TEXT("AFortVolumeManager::CanCreatePlayset: No. [VolumeManager:%s, Thrower:%s, NewPlayset:%s]"), *GetNameSafe(VolumeManager), *GetNameSafe(Thrower), *GetNameSafe(NewPlayset));
    return false;
}

AFortVolume* LocationVolume = FindVolumeForLocation(VolumeManager, StartLocation);
AFortVolume* ThrowerVolume = nullptr;
AFortPlayerControllerAthena* ThrowerController = Cast<AFortPlayerControllerAthena>(Thrower->Controller);
const bool bPlayerCanCreateInVolume = ThrowerController != nullptr && NativeCanPlayerCreateInCurrentVolume(ThrowerVolume);

if (bPlayerCanCreateInVolume && ThrowerVolume == LocationVolume && LocationVolume != nullptr)
{
    if (NativeCanSpawnVolume(NewPlayset, StartLocation, PlaysetRotation))
    {
        UE_LOG(LogFortVolume, Verbose, TEXT("AFortVolumeManager::CanCreatePlayset: Yes."));
        return true;
    }

    UE_LOG(LogFortVolume, Verbose, TEXT("AFortVolumeManager::CanCreatePlayset: No. Playset is not within volume bounds"));
    return false;
}

UE_LOG(LogFortVolume, Verbose, TEXT("AFortVolumeManager::CanCreatePlayset: No. [bPlayerCanCreateInVolume:%d, ThrowerVolume == LocationVolume:%d, LocationVolume != nullptr:%d]"), bPlayerCanCreateInVolume, ThrowerVolume == LocationVolume, LocationVolume != nullptr);
return false;
