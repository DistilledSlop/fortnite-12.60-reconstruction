// Original address: 0x144c21700
// Evidence: [this+0x1d0] = PlayerState (reflected AFortPlayerStateAthena*). Null -> AL=0 (false). Else byte [PlayerState+0xd00] & 1 = bInAircraft (bitfield mask 0x1, AFortPlayerStateZone).
// CONFIDENCE: HIGH
if (PlayerState == nullptr)
{
	return false;
}
return PlayerState->bInAircraft;
