// CONFIDENCE: MEDIUM
// Original: 0x1423ac760 (virtual Implementation of ServerSetInventoryStateValue). Callees not named here are described in the evidence file.
// Authority-only path: when this controller is not the authority, forward the call through the remote-call function.
if (GetLocalRole() != ROLE_Authority)
{
	ProcessEventByName(NativeFName_0x1481af270, ItemGuid, StateValue);
}
