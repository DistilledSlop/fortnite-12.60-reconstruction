// Native 0x141a470e0, log string names it AFortPoiManager::GetPoiGridTagsForLocation. CONFIDENCE: MEDIUM.
// Converts Location to a grid cell using GridOrigin (+0x218/+0x21c), GridCellSize (+0x228/+0x22c), GridWidth (+0x230) and GridHeight (+0x234); cell indices are clamped to [0, size-1]. The cell maps to a slot in the cell->tag table (+0x2e0, count +0x2e8). When the grid is disabled (+0x2c4 != 0) or the cell size is degenerate, nothing is appended.
// The appended tag entry is at slot*0x20 + 0x2b0 of the tag table (count +0x2b8).
int32 CellIndex = -1;
if (!bGridDisabled)
{
	if (GridCellSize.X > 1e-8f && GridCellSize.Y > 1e-8f)
	{
		int32 CellX = FMath::Max(0, (int32)((float)(int32)(Location.X - GridOrigin.X) / GridCellSize.X));
		CellX = CellX < GridWidth ? CellX : GridWidth - 1;
		int32 CellY = FMath::Max(0, (int32)((float)(int32)(Location.Y - GridOrigin.Y) / GridCellSize.Y));
		CellY = CellY < GridHeight ? CellY : GridHeight - 1;
		CellIndex = CellY * GridWidth + CellX;
	}
}
if (CellIndex >= 0 && CellIndex < GridCellTagIndices.Num())
{
	int16 Slot = GridCellTagIndices[CellIndex];
	if (TagTable.Num() > 0 && Slot >= 0 && Slot < TagTable.Num())
	{
		OutGameplayTags.AppendTags(TagTable[Slot]);
		if (LogVerbosityAtLeast(7))
		{
			UE_LOG(LogFortnite, Verbose, TEXT("AFortPoiManager::GetPoiGridTagsForLocation - PoiGrid is appending %s tags to OutGameplayTags for (%0.5f, %0.5f)"), *TagTable[Slot].ToString(), Location.X, Location.Y);
		}
	}
}
