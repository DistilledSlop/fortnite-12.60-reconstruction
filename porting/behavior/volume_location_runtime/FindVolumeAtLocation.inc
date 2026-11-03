// CONFIDENCE: HIGH
QUICK_SCOPE_CYCLE_COUNTER(STAT_Find_Volume);
// 0x141c7cde0. In the X comparison unordered values enter the Y comparison.
AFortVolume* Selected = nullptr;
FBoxSphereBounds CandidateBounds;
FBoxSphereBounds SelectedBounds;
for (AFortVolume* Candidate : VolumeObjects)
{
    if (Candidate && Candidate->NativeContainsLocation(Location))
    {
        if (!Selected)
        {
            Selected = Candidate;
        }
        else
        {
            Candidate->NativeGetLocationBounds(CandidateBounds);
            Selected->NativeGetLocationBounds(SelectedBounds);
            if (!(CandidateBounds.BoxExtent.X >= SelectedBounds.BoxExtent.X))
            {
                Candidate->NativeGetLocationBounds(SelectedBounds);
                Selected->NativeGetLocationBounds(CandidateBounds);
                if (CandidateBounds.BoxExtent.Y > SelectedBounds.BoxExtent.Y)
                {
                    Selected = Candidate;
                }
            }
        }
    }
}
return Selected;
