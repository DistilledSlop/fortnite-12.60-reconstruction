// Original body 0x144cc6880 (thunk 0x1451e1880). Offset evidence: SUB RCX,-0x80 (= +0x80) -> OnPrecisionChanged (reflected exact at +0x80). Pack: [0]=DL bPrecisionOn, [1]=R8B GridSnapIndex.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>: mapped to <Member>.Broadcast(...) per the broadcast addendum.
// CONFIDENCE: HIGH
OnPrecisionChanged.Broadcast(bPrecisionOn, GridSnapIndex);
