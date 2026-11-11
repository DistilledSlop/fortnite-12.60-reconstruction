// CONFIDENCE: MEDIUM. Clears the building grid cells listed in GridCellsToClear. Each entry is an offset from BaseDestination, rotated by AngleDeg about the vertical axis. Every cell is cleared through DestroyActorsInGridCellLocation inside one structural-grid update. Nothing happens when the world has no structural grid.
// Original addresses: exec 0x142aeeff0 (thunk; calls the body 0x141a71c60 directly with the same arguments). Helpers: 0x141c77490 (GetStructuralGridForWorld), 0x141c91940 and 0x141c91e20 (update begin and end, names proposed), 0x140692270 (vector rotate about the given axis).

UStructuralGrid* StructuralGrid = GetStructuralGridForWorld(GetWorld());
if (StructuralGrid == nullptr)
{
    return;
}

StructuralGrid->BeginBatchUpdate();
for (const FVector& CellOffset : GridCellsToClear)
{
    const FVector RotatedOffset = CellOffset.RotateAngleAxis(AngleDeg, FVector(0.0f, 0.0f, 1.0f));
    const FVector CellLocation = BaseDestination + RotatedOffset;
    DestroyActorsInGridCellLocation(bDestroyFloorsQuickly, CellLocation, StructuralGrid);
}
StructuralGrid->EndBatchUpdate();
