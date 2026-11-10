// CONFIDENCE: HIGH
// Original address: 0x142a71050 (exec thunk). Thunk read order: PlayerToSpectate (object), then bAllowStateChange (bool).
// _Validate: vtable slot 0x2158 -> 0x1406801d0 = STUB (returns true), so validation never rejects.
// _Implementation: vtable slot 0x2160 -> 0x1423e98c0 (ported in AFortPlayerControllerZone__ServerSpectatePlayerState_Implementation.inc).
if (!ServerSpectatePlayerState_Validate(PlayerToSpectate, bAllowStateChange))
{
	RPC_ValidateFailed(TEXT("ServerSpectatePlayerState_Validate"));
	return;
}
ServerSpectatePlayerState_Implementation(PlayerToSpectate, bAllowStateChange);
