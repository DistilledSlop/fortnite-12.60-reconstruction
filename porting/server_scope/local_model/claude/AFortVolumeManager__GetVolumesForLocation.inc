// CONFIDENCE: MEDIUM Original addresses: thunk 0x142b3c2f0; see evidence.json for callees.
// Signature: bool GetVolumesForLocation(const FVector& Location, TArray<AFortVolume*>& Volumes)
QUICK_SCOPE_CYCLE_COUNTER(STAT_Find_Volume);
Volumes.Reset();
for (AFortVolume* Candidate : VolumeObjects)
{
    if (Candidate && Candidate->NativeContainsLocation(Location))
    {
        Volumes.AddUnique(Candidate);
    }
}
return Volumes.Num() > 0;
