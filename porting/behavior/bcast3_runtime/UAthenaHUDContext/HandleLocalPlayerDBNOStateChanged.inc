// Original body 0x144c936e0 (thunk 0x14519e7e0). Offset evidence: ADD RCX,0x98 -> OnLocalPlayerDBNOStateChanged (reflected exact at +0x98). Pack: [0]=R8B bIsDBNO; RDX (FortPawn) never stored.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>: mapped to <Member>.Broadcast(...) per the broadcast addendum.
// CONFIDENCE: MEDIUM
OnLocalPlayerDBNOStateChanged.Broadcast(bIsDBNO);
