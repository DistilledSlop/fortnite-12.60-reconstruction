// CONFIDENCE: MEDIUM - the set-bit walk over the building's support mask and the BuildingSMActor class filter are read in full; member names are proposed.
// Original addresses: exec 0x14275c550; body 0x141c78cf0
LogicalBuildingGroundSupportedActors.Reset();
if (!LogicalBuildings.IsValidIndex(LogicalIndex))
{
    return;
}
const FStructuralLogicalBuilding& Building = LogicalBuildings[LogicalIndex];
for (int32 CellIndex = 0; CellIndex < Building.GroundSupportMask.Num(); ++CellIndex)
{
    if (!Building.GroundSupportMask[CellIndex])
    {
        continue;
    }
    ABuildingSMActor* Actor = Cast<ABuildingSMActor>(Building.SupportCells[CellIndex].Actor.Get());
    if (Actor != nullptr)
    {
        LogicalBuildingGroundSupportedActors.Add(Actor);
    }
}
