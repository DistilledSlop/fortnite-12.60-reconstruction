// Original body 0x141b26ba0 (thunk 0x142827fa0). Offset evidence: ADD RCX,0x980 -> OnTeamsRolesChanged (reflected exact at +0x980). Pack: [0]=DL NewPusherTeam, [1]=R8B NewDefenerTeam.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>: mapped to <Member>.Broadcast(...) per the broadcast addendum.
// CONFIDENCE: HIGH
OnTeamsRolesChanged.Broadcast(NewPusherTeam, NewDefenerTeam);
