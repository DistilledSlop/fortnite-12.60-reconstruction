// CONFIDENCE: high; native stable remove-all helper140726fc0 preserves allocation.
if (VolumesLoading.Contains(VolumeToLoad))
{
    VolumesLoading.Remove(VolumeToLoad);
    return;
}
AFortVolume* const MissingVolume = VolumeToLoad;
UE_LOG(LogFort, Warning, TEXT("Server End Loading volume called for volume not currently loading %s"),
    MissingVolume ? *MissingVolume->GetName() : TEXT("None"));
