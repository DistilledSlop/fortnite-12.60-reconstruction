// Original body address: 0x141e3c260 (thunk 0x142882370)
// Offset evidence: byte [RCX+0x744] = bNotifyUsersWhenPlaying (reflected bool bit); on change the asm writes byte [0x744] = DL (Value, arg1),
// byte [0x87e] = 1 (bDirty, reflected bool bit) and byte [0x745] = 1 (NativeRecoveredBool_0x745, unreflected gap).
// CONFIDENCE: MEDIUM
if (bNotifyUsersWhenPlaying != Value)
{
    bNotifyUsersWhenPlaying = Value;
    bDirty = true;
    NativeRecoveredBool_0x745 = true;
}
