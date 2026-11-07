// Behaviour snippet: AFortGameModeZone::SetEnemyAIItemCacheLootDrops (body only; spliced into the generated member function).
// Original address: 0x141eaed80
// Evidence: Dword [RCX+0x820] bit 1 (mask 0x2) cleared then set from bValue.
// Evidence: Reflected bitfield bAllowEnemyAIItemCacheLootDrops (offset 2080, mask 2).
// CONFIDENCE: HIGH
bAllowEnemyAIItemCacheLootDrops = bValue;
