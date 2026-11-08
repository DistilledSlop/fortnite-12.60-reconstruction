// Helper 0x1423cb760 (AFortWeaponRanged). Asm: CMP byte ptr [RCX+0x2670],0x2 ; SETZ AL ; RET.
// Offset evidence: no reflected member covers 0x2670 (lookup_offset.py AFortWeaponRanged 0x2670 empty), so raw uint8 access is used. Meaning of the value 2 is not established.
// Caller evidence: UseScopeTargeting @0x1426e77c0 passes its weapon in RCX; SetMaterialParameters @0x141e7f0a0 calls it after an IsA-style class-tree check.
// CONFIDENCE: MEDIUM

return *reinterpret_cast<const uint8*>(reinterpret_cast<const uint8*>(WeaponObj) + 0x2670) == 2;
