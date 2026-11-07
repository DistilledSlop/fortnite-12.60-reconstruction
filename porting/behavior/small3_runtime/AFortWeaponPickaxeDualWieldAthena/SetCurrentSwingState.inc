// Original body: 0x141a8c510 (thunk 0x142b4ff50)
// Offset evidence: CMP byte [RCX+0xc89],0; JNZ ret; MOV byte [RCX+0xc88],DL
// CONFIDENCE: HIGH
if (!NativeRecoveredBool_0xc89)
{
    NativeRecoveredUint8_0xc88 = static_cast<uint8>(NewState);
}
