// Original body 0x1425d8a40 (thunk 0x1428dbb50). Offset evidence: ADD RCX,0x60 -> OnTimedOut (reflected exact at +0x60). XOR EDX: no params.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>: mapped to <Member>.Broadcast(...) per the broadcast addendum.
// CONFIDENCE: HIGH
OnTimedOut.Broadcast();
