// Original body 0x144c20230 (thunk 0x1451b8930). Offset evidence: MOV [RCX+0x1e8],DL: store to unreflected gap in own region (last own member PlayerState at +0x1d0). ADD RCX,0x48 -> OnSquadIdChanged (exact). Pack: [0]=this, [8]=DL.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>: mapped to <Member>.Broadcast(...) per the broadcast addendum.
// CONFIDENCE: HIGH
NativeRecoveredByte_0x1e8 = InSquadId;
OnSquadIdChanged.Broadcast(this, InSquadId);
