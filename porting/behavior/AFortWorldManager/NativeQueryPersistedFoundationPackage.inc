// CONFIDENCE: HIGH. Original 0x141f8a060.
// CurrentZoneRecord is paired with CurrentWorldRecord by the native lifecycle.
AWorldTileFoundation* Tile = Cast<AWorldTileFoundation>(Foundation);
if (!CurrentWorldRecord || !Foundation)
    return NAME_None;
if (GetWorld()->WorldType == EWorldType::PIE)
    return NAME_None;
if (!Foundation->MyGuid.IsValid() && !Tile)
    return NAME_None;
UFortZoneRecord* Zone = CurrentZoneRecord;
int32 ParentIndex = INDEX_NONE;
if (Zone->NativeFindActorLevelRecord(Foundation, &ParentIndex))
{
    for (const FFortLevelRecord& Record : Zone->SavedLevels)
    {
        if (Record.ParentLevelIndex != ParentIndex)
            continue;
        if (Tile)
        {
            if (Record.X_Loc != Tile->NativeTileX_0xee4 ||
                Record.Y_Loc != Tile->NativeTileY_0xee8)
                continue;
        }
        else if (Record.BoundActorGuid != Foundation->MyGuid)
        {
            continue;
        }
        return Record.PackageName;
    }
}
return NAME_None;
