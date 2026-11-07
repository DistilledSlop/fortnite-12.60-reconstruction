// Original body 0x144d74f20 (thunk 0x14286bb50). Offset evidence: ADD RCX,0x298 -> OnOpenedOrClosed (reflected exact at +0x298). Pack: [0]=MOV byte 0 (false).
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>: mapped to <Member>.Broadcast(...) per the broadcast addendum.
// CONFIDENCE: HIGH
OnOpenedOrClosed.Broadcast(false);
