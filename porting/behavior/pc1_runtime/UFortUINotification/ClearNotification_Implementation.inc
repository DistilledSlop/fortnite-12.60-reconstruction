// Original body 0x1425e87d0 (thunk 0x142710860). Offset evidence: dword [+0x98] is the invocation-list Num inside OnNotificationCleared (+0x90, reflected InlineMulticastDelegate); byte [+0xc9] = NativeRecoveredBool_0xc9; Broadcast packs this as the single UFortUINotification* argument.
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
if (*reinterpret_cast<const int32*>(Raw + 0x98) > 0)
{
    NativeRecoveredBool_0xc9 = false;
    OnNotificationCleared.Broadcast(this);
}
