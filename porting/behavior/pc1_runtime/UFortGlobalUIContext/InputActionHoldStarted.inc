// Original body 0x144cd66d0 (thunk 0x145249f60). Offset evidence: dword [+0x90] is the invocation-list Num inside OnInputActionHoldStarted (+0x88, reflected InlineMulticastDelegate); Broadcast packs {InputActionName (+0), HoldDuration (+8)} as FOnInputActionHoldStarted arguments (XMM2 = HoldDuration).
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
if (*reinterpret_cast<const int32*>(Raw + 0x90) > 0)
{
    OnInputActionHoldStarted.Broadcast(InputActionName, HoldDuration);
}
