// Original body 0x141f3f250 (thunk 0x142aa2620). Offset evidence: MOVZX EAX,[RCX+0x221]: LinkCodeLockStatus (reflected exact). ADD RCX,0xe0 -> OnPortalLinkCodeLockStatusChanged (exact). Pack: [0]=AL.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>: mapped to <Member>.Broadcast(...) per the broadcast addendum.
// CONFIDENCE: HIGH
OnPortalLinkCodeLockStatusChanged.Broadcast(LinkCodeLockStatus);
