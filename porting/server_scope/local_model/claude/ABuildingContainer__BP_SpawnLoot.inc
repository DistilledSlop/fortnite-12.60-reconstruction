// CONFIDENCE: LOW. Chunk 01 port. See ABuildingContainer__BP_SpawnLoot.evidence.json for captures and notes.
// Original address: 0x14273c1f0 (exec). Direct callee 0x141bd5720 forwards to virtual slot 0xee0 (ABuildingContainer::SpawnLoot, base stub at 0x141bec430).
// The base SpawnLoot logs "ABuildingContainer::SpawnLoot() called on ..." and returns false, so the real result comes from the derived override.
// The trailing (4, false, false) literals are passed by the thunk; their meaning is not recovered.
return SpawnLoot(PlayerPawn, 4, false, false);
