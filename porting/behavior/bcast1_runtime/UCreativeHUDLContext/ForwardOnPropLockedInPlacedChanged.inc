// Behaviour snippet: UCreativeHUDLContext::ForwardOnPropLockedInPlacedChanged (wrapper body; spliced into the generated member function).
// Original body address: 0x144cc68a0 (thunk 0x1451e1940, receiver delta 0, this delta total 0).
// Offset evidence: MOV byte [RCX+0x148],DL before the ADD stores bLockedInPlace into reflected bool bPropLockedInPlace (byte +0x148, mov8 form).
// Offset evidence: ADD RCX,0x130 -> OnPropLockInPlaceChanged (reflected at +0x130; FOnGenericBooleanStateChangedEvent(bool bStateOn)); DL copied to param offset 0.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: HIGH
bPropLockedInPlace = bLockedInPlace;
OnPropLockInPlaceChanged.Broadcast(bLockedInPlace);
