// Original body 0x141c7f310 (thunk 0x142b3c8f0). Offset evidence: ADD RCX,0x268 -> OnVolumeStateChanged (reflected exact at +0x268). Pack: [0]=DL NewState, [8]=R8 Volume.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>: mapped to <Member>.Broadcast(...) per the broadcast addendum.
// CONFIDENCE: HIGH
OnVolumeStateChanged.Broadcast(NewState, Volume);
