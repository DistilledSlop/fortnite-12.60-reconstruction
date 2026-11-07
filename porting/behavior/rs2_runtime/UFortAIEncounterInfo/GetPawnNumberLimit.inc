// Original body address: 0x14165db30 (thunk 0x1427984a0)
// Offset evidence: +0xb44 and +0xb40 are unreflected gaps (member_field NativeRecoveredByte_0xb44 and _0xb40).
// +0x27c SpawnLimitType (Byte), +0x280 SpawnLimit (Int), +0xd4 PawnNumberLimit (Int) are reflected.
// Jump table: JZ targets (b44==1, b40==2, b44==3) return PawnNumberLimit; else SpawnLimitType==1 returns SpawnLimit; else -1.
// CONFIDENCE: HIGH
if (NativeRecoveredByte_0xb44 == 1 || NativeRecoveredByte_0xb40 == 2 || NativeRecoveredByte_0xb44 == 3)
{
	return PawnNumberLimit;
}
if (static_cast<uint8>(SpawnLimitType) == 1)
{
	return SpawnLimit;
}
return -1;
