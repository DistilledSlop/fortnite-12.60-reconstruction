// CONFIDENCE: MEDIUM - grid lookup and the filter/collect split read in full; the out struct field names are proposed.
// Original addresses: exec 0x14275d760; body 0x141c75940 (+0x141c78270, 0x141c757a0)
OutActorsInGridCell = FBuildingGridCellActors();
FIntVector Cell;
if (!NativeWorldLocationToGridCell(WorldLocation, Cell))
{
    return false;
}
return NativeCollectGridCellActors(Cell, Filter, OutActorsInGridCell);
