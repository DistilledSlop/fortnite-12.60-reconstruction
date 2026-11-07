// Behaviour snippet: AFortVolume::OnRep_UserGeneratedContentRestrictions (wrapper body; spliced into the generated member function).
// Original body address: 0x141c840c0 (thunk 0x142b37d20, receiver delta 0, this delta total 0).
// Offset evidence: CMP byte [RCX+0x5c8],0 then SETNZ into param offset 0: reflected plain bool bUserGeneratedContentRestricted (bool_bits byte +0x5c8); byte != 0 equals the bool value.
// Offset evidence: ADD RCX,0x5b8 -> OnUserGeneratedContentRestrictionChanged (reflected at +0x5b8; FOnRestrictionEnabledChanged(bool bEnabled)).
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: HIGH
OnUserGeneratedContentRestrictionChanged.Broadcast(bUserGeneratedContentRestricted);
