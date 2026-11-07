// Original body address: 0x141e2c590 (thunk 0x14287e720)
// Offset evidence: byte [RCX+0x555] = bEnableVoiceChat; byte [RCX+0x8a8] = NativeRecoveredBool_0x8a8; byte [RCX+0x558] = bCanVoiceChatWithUnknowns (reflected bool bits, mask 1).
// Asm: gated off -> 0 (Off); else SETNZ(byte 0x558) + INC -> 2 (Everyone) or 1 (FriendsOnly).
// CONFIDENCE: MEDIUM
if (!bEnableVoiceChat || NativeRecoveredBool_0x8a8)
{
    return ESelectableVoiceChatSetting::Off;
}
return bCanVoiceChatWithUnknowns ? ESelectableVoiceChatSetting::Everyone : ESelectableVoiceChatSetting::FriendsOnly;
