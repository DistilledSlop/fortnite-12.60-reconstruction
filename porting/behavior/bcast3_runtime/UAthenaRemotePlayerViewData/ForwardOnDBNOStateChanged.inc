// Original body 0x144c1fe50 (thunk 0x1451b8230). Offset evidence: ADD RCX,0xc8 -> OnDBNOStateChanged (reflected exact at +0xc8). Pack: [0]=RCX before ADD (this), [8]=R8B bInIsDBNO.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>: mapped to <Member>.Broadcast(...) per the broadcast addendum.
// CONFIDENCE: MEDIUM
OnDBNOStateChanged.Broadcast(this, bInIsDBNO);
