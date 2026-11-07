// Original body 0x144cd3d70 (thunk 0x145257450). Offset evidence: ADD RCX,0x4d0 -> OnBuildModeChanged (reflected exact at +0x4d0). Pack: [0]=DL bool.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>: mapped to <Member>.Broadcast(...) per the broadcast addendum.
// CONFIDENCE: HIGH
OnBuildModeChanged.Broadcast(bEntering);
