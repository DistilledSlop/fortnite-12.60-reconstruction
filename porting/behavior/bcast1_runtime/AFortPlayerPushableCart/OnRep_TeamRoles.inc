// Behaviour snippet: AFortPlayerPushableCart::OnRep_TeamRoles (wrapper body; spliced into the generated member function).
// Original body address: 0x141a53240 (thunk 0x142a89760, receiver delta 0, this delta total 0).
// Offset evidence: MOVZX/MOV of byte [RCX+0x829] (TeamRoles FTeamRoles.PusherTeam at +0) and byte [RCX+0x82a] (DefenderTeam at +1) stored at param offsets 0 and 1.
// Offset evidence: ADD RCX,0x7f0 -> PlayerPushableCartOnTeamsRolesChanged (reflected at +0x7f0; FPlayerPushableCartOnTeamsRolesChanged(uint8 PusherTeam, uint8 DefenderTeam)).
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: HIGH
PlayerPushableCartOnTeamsRolesChanged.Broadcast(TeamRoles.PusherTeam, TeamRoles.DefenderTeam);
