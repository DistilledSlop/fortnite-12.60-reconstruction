// CONFIDENCE: MEDIUM Original addresses: thunk 0x142b3d710; see evidence.json for callees.
// Signature: AFortVolume* SpawnVolume(TSubclassOf<AFortVolume> VolumeActor, UObject* Playset, const FVector& Location, const FRotator& Rotation)
// Helpers proposed: NativeCanSpawnVolume, NativeInitVolumePlayset, NativeRegisterVolume (see deps).
if (Role != ROLE_Authority)
{
    return nullptr;
}
UWorld* World = GetWorld();
if (!World || !VolumeActor || !VolumeActor->IsChildOf(AFortVolume::StaticClass()))
{
    return nullptr;
}
if (!NativeCanSpawnVolume(Playset, Location, Rotation))
{
    return nullptr;
}
const FTransform SpawnTransform(Rotation.Quaternion(), Location, FVector::OneVector);
FActorSpawnParameters SpawnParameters;
SpawnParameters.OverrideLevel = GetLevel();
SpawnParameters.bDeferConstruction = true;
AFortVolume* Volume = World->SpawnActor<AFortVolume>(VolumeActor, SpawnTransform, SpawnParameters);
if (!Volume)
{
    return nullptr;
}
NativeInitVolumePlayset(Volume, Playset);
Volume->FinishSpawning(SpawnTransform);
NativeRegisterVolume(Volume);
return Volume;
