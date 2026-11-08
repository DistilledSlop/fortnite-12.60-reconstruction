// Original body address: 0x1418f7290 (thunk 0x1427d8490, RPC implementation)
// Offset evidence: arg InControlState (FReplicatedControlState, 28 bytes) is copied to this+0x1554 (reflected ControlState) as 16+8+4 bytes.
// this+0xe0 is AActor::Owner (private in UE 4.25, so GetOwner()); when it is null, this+0x1245 (unreflected bool gap, NativeRecoveredBool_0x1245) = arg bit0 at +0x18 (bIsEngineOn).
// The JNZ at 0x1418f72b8 targets the RET, so the bool store runs only when Owner is null.
// CONFIDENCE: HIGH
ControlState = InControlState;
if (GetOwner() == nullptr)
{
	NativeRecoveredBool_0x1245 = InControlState.bIsEngineOn;
}
