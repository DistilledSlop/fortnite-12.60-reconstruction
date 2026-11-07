// Behaviour snippet: UAthenaRemotePlayerViewData::ForwardOnWeaponChanged (wrapper body; spliced into the generated member function).
// Original body address: 0x144c205b0 (thunk 0x1451b8ad0, receiver delta 0, this delta total 0).
// Offset evidence: MOV [RSP+0x20],RCX stores the receiver (this) at param offset 0; MOV [RSP+0x28],RDX stores InNewWeapon at offset 8; R8 (InPrevWeapon) is never read.
// Offset evidence: ADD RCX,0x118 -> OnWeaponChanged (reflected at +0x118; FWeaponDelegate(UAthenaRemotePlayerViewData* PlayerData, AFortWeapon* WeaponParam)).
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: MEDIUM
OnWeaponChanged.Broadcast(this, InNewWeapon);
