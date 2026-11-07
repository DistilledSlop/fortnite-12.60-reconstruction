// Original body address: 0x144c22bb0 (thunk 0x1451b8fe0, group B, 13 instructions).
// Offset evidence: this+0x21c byte is an unreflected gap in the class's own region (member NativeRecoveredBool_0x21c, bool).
// this+0x204 int32 is an unreflected gap in the class's own region (member NativeRecoveredInt32_0x204).
// this+0x1d0 is the reflected PlayerState (AFortPlayerStateAthena*); PlayerState+0xe6c is the reflected Place (Int) per lookup_offset.py AFortPlayerStateAthena.
// CONFIDENCE: HIGH
if (NativeRecoveredBool_0x21c && NativeRecoveredInt32_0x204 == 0x3e7)
{
	return 0x3e7;
}
if (PlayerState)
{
	return PlayerState->Place;
}
return NativeRecoveredInt32_0x204;
