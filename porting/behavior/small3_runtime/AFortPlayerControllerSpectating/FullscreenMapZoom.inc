// Original body: 0x1423d2be0 (thunk 0x142a6b340)
// Offset evidence: MOVSS [RCX+0x2ac8],XMM1 after UCOMISS(Value,0); JZ skips store on equal or unordered
// CONFIDENCE: HIGH
if (Value != 0.0f && Value == Value)
{
    NativeRecoveredFloat_0x2ac8 = Value;
}
