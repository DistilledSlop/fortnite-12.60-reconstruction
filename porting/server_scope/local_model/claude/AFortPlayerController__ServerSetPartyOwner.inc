// CONFIDENCE: HIGH
// Original: exec RPC wrapper 0x142a4d5e0. Validate slot -> 0x1406801d0. Implementation slot -> 0x1423ac960.
// Single unique-net-id struct; the thunk releases its shared-pointer refcount, which RAII covers here.
if (!ServerSetPartyOwner_Validate(PartyOwnerUniqueId))
{
	RPC_ValidateFailed(TEXT("ServerSetPartyOwner_Validate"));
	return;
}
ServerSetPartyOwner_Implementation(PartyOwnerUniqueId);
