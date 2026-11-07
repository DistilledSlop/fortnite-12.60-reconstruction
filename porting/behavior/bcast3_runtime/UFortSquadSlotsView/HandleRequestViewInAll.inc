// Original body 0x145127e00 (thunk 0x1452ec880). Offset evidence: ADD RCX,0x288 -> OnRequestViewInAllEvent (reflected exact at +0x288). Pack: [0]=EDX dword.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>: mapped to <Member>.Broadcast(...) per the broadcast addendum.
// CONFIDENCE: HIGH
OnRequestViewInAllEvent.Broadcast(SquadSlotIndex);
