// Original body 0x144cc68d0 (thunk 0x1451e1a50). Offset evidence: ADD RCX,0x40 -> OnTemporarySpecialUIEvent (reflected exact at +0x40). Pack: [0]=DL enum byte. Shared body with UCreativeHUDLContext::ForwardOnScaleAxisChanged.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>: mapped to <Member>.Broadcast(...) per the broadcast addendum.
// CONFIDENCE: MEDIUM
OnTemporarySpecialUIEvent.Broadcast(InNewTemporaryEvent);
