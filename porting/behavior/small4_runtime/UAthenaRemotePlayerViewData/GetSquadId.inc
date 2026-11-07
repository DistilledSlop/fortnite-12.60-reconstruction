// Original address: 0x144c23c40
// Evidence: [this+0x1d0] = PlayerState. Non-null -> byte PlayerState->SquadId (+0xfdc). Null -> byte at this+0x1e8 (unreflected, NativeRecoveredUint8_0x1e8).
// CONFIDENCE: HIGH
if (PlayerState != nullptr)
{
	return PlayerState->SquadId;
}
return NativeRecoveredUint8_0x1e8;
