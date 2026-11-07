// Original address: 0x144c21890
// Evidence: [this+0x1d0] = PlayerState. Non-null -> int32 PlayerState->KillScore (+0xe74). Null -> int32 at this+0x200 (unreflected, NativeRecoveredInt32_0x200).
// CONFIDENCE: HIGH
if (PlayerState != nullptr)
{
	return PlayerState->KillScore;
}
return NativeRecoveredInt32_0x200;
