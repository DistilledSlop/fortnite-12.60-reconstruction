// Behaviour snippet: AFortPlayerControllerSpectating::SetNotificationsEnabled (body only; spliced into the generated member function).
// Original body address: 0x1423ebdb0 (thunk 0x142a6cce0, group B, flags 0x4020401).
// Offset evidence: this+0x2bf0 = flag byte with no reflected member (bool_bits has no entry; member_field NativeRecoveredUint8_0x2bf0 added), bit 0x2 = notifications-enabled state. this+0x27b0 = OnShowNotificationsEnabledChanged (reflected delegate, one bool parameter). The asm returns early when bit 1 already equals the argument.
// CONFIDENCE: HIGH
if (((NativeRecoveredUint8_0x2bf0 >> 1) & 1) != static_cast<uint8>(bNewState))
{
    NativeRecoveredUint8_0x2bf0 = static_cast<uint8>((NativeRecoveredUint8_0x2bf0 & 0xfd) | (static_cast<uint8>(bNewState) << 1));
    OnShowNotificationsEnabledChanged.Broadcast(bNewState);
}
