// Original body 0x144cc6130 (thunk 0x1451e15d0). Offset evidence: ADD RCX,0x110 -> OnCreativeQuickbarEquippedChanged (reflected exact at +0x110). Pack: [0]=DL bool.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>: mapped to <Member>.Broadcast(...) per the broadcast addendum.
// CONFIDENCE: HIGH
OnCreativeQuickbarEquippedChanged.Broadcast(bIsEquipped);
