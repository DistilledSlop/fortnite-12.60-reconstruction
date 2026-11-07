// Original body 0x141a4e410 (thunk 0x142a89550). Offset evidence: MOVZX EAX,[RCX+0x828]: CurrentCheckPoint (reflected exact, uint8). ADD RCX,0x790 -> OnCheckPointReached (exact). Pack: MOV dword [0]=EAX.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>: mapped to <Member>.Broadcast(...) per the broadcast addendum.
// CONFIDENCE: HIGH
OnCheckPointReached.Broadcast(static_cast<int32>(CurrentCheckPoint));
