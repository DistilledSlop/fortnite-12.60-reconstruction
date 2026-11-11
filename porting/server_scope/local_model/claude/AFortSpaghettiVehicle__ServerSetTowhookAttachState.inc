// CONFIDENCE: HIGH. Server RPC storing the client's towhook attach state. Validation is the engine stub that always passes, so the ValidateFailed path is unreachable. The implementation stores the incoming state as the replicated attach state and clears the companion slot to default first.
// Original addresses: exec 0x142a09490 (thunk: _Validate via virtual 0xeb8, then _Implementation via virtual 0xec0). Validate target 0x1406801d0 (returns true). Implementation target 0x1419f2760.

if (!ServerSetTowhookAttachState_Validate(InTowhookAttachState))
{
    RPC_ValidateFailed(TEXT("ServerSetTowhookAttachState_Validate"));
    return;
}

// Companion slot at +0x1170 is cleared to a default-constructed NetTowhookAttachState (name unresolved, LOW).
LastAppliedTowhookAttachState = FNetTowhookAttachState();
ReplicatedAttachState = InTowhookAttachState;
