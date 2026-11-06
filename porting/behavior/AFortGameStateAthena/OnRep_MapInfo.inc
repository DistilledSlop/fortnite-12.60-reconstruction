// CONFIDENCE: HIGH -- original exec 0x142920510, target 0x1419a68a0.
const AFortAthenaMapInfo* CurrentMapInfo = MapInfo;
if (CurrentMapInfo)
{
    LootInfo.TreasureChestSpawnInfos = CurrentMapInfo->TreasureChestSpawnInfos;
    LootInfo.AmmoBoxSpawnInfos = CurrentMapInfo->AmmoBoxSpawnInfos;
}
