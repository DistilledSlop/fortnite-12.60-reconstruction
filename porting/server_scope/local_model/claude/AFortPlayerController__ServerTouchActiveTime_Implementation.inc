// CONFIDENCE: MEDIUM
// Original: 0x1423ad120 (virtual Implementation of ServerTouchActiveTime). Callees not named here are described in the evidence file.
// Record activity for the idle check and clear the pending idle-warning flag.
NativeRecordActivityTimestamp();
bIdleWarningPending = false;
