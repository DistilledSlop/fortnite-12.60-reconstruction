// Original body 0x144cc60b0 (thunk 0x1451e1390). Offset evidence: ADD RCX,0x60 -> OnCreativeHeatmapToggled (reflected exact at +0x60). Pack: [0]=DL bool.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>: mapped to <Member>.Broadcast(...) per the broadcast addendum.
// CONFIDENCE: HIGH
OnCreativeHeatmapToggled.Broadcast(bIsHeatmapToggled);
