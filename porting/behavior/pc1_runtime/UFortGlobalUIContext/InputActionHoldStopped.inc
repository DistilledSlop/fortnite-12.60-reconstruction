// Original body 0x144cd6700 (thunk 0x14524a030). Offset evidence: dword [+0xa0] is the invocation-list Num inside OnInputActionHoldStopped (+0x98, reflected InlineMulticastDelegate); Broadcast packs {InputActionName (+0), bCompletedSuccessfully (+8)} as FOnInputActionHoldStopped arguments (R8B = bool).
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
if (*reinterpret_cast<const int32*>(Raw + 0xa0) > 0)
{
    OnInputActionHoldStopped.Broadcast(InputActionName, bCompletedSuccessfully);
}
