// CONFIDENCE: HIGH for control flow; validate (vcall 0xe78) is the return-true stub 0x1406801d0. Impl 0x14234a990 calls CanRespawnFromDBNO (vcall 0x11e0) and only then RespawnFromDBNOInternal (vcall 0x1260). On AFortPlayerPawnAthena slot 0x11e0 is the return-false stub 0x1406801e0 (XOR AL,AL; RET), so the respawn is a no-op on Athena pawns; the base AFortPlayerPawn slot 0x14230c2f0 was not read.
if (!CanRespawnFromDBNO())
{
	return;
}
RespawnFromDBNOInternal();
