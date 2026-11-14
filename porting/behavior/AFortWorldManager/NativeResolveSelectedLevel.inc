// CONFIDENCE: HIGH; source-complete under native initialization preconditions.
// Original 0x141f89870. The append helper returns a pointer into SavedLevels.
if (!CurrentWorldRecord || !CurrentZoneRecord)
    return NAME_None;
UFortZoneRecord* Zone = CurrentZoneRecord;
FFortLevelRecord* Record = Zone->NativeAddSavedLevel(SelectedBuilding, Foundation);
if (!Record)
    return NAME_None;
if (AWorldTileFoundation* Tile = Cast<AWorldTileFoundation>(Foundation))
{
    Record->X_Loc = Tile->NativeTileX_0xee4;
    Record->Y_Loc = Tile->NativeTileY_0xee8;
    Record->Rotation = Tile->NumRotations;
}
return Record->PackageName;
