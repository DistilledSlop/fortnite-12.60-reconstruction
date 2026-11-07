// Original body 0x141f3f550 (thunk 0x1429f0500). Offset evidence: CMP dword [RCX+0xc0],0; JLE skip: Num of OnMissionTimerComponentUpdated (+0xb8) > 0. Pack: [0]=RCX this.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>: mapped to <Member>.Broadcast(...) per the broadcast addendum.
// CONFIDENCE: HIGH
if (OnMissionTimerComponentUpdated.IsBound())
{
    OnMissionTimerComponentUpdated.Broadcast(this);
}
