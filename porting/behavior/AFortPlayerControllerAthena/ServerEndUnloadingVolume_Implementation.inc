// CONFIDENCE: high; native stable remove-all helper140726fc0 preserves allocation.
if (VolumesUnloading.Contains(VolumeToUnload))
{
    VolumesUnloading.Remove(VolumeToUnload);
    return;
}
AFortVolume* const MissingVolume = VolumeToUnload;
UE_LOG(LogFort, Warning, TEXT("Server End Unloading volume called for volume not currently loading %s"),
    MissingVolume ? *MissingVolume->GetName() : TEXT("None"));
