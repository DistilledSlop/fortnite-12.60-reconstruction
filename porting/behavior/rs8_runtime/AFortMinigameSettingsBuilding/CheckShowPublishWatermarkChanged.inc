// Original body address: 0x1426f9920
// Offset evidence: byte at +0x258 (no reflected member; NativeRecoveredUint8_0x258) compared with byte at +0x320 (not covered by SavedActorGuid FGuid +0x310..+0x31f; NativeRecoveredUint8_0x320). Delegate at +0x290 gets (this, bool) on change.
// CONFIDENCE: HIGH
const uint8 Current = NativeRecoveredUint8_0x258;
if (Current == NativeRecoveredUint8_0x320) { return; }
NativeRecoveredUint8_0x320 = Current;
OnShowPublishWatermarkChanged.Broadcast(this, Current != 0);
