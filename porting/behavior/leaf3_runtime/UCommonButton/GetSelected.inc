// Behaviour snippet: UCommonButton::GetSelected (body only; spliced into the generated member function).
// Original body address: 0x1407c4890 (thunk 0x14080b350)
// Offset evidence: MOVZX EAX,byte [this+0x2a1]; AND AL,1 (bit 0, mask 0x01). New byte member NativeRecoveredBits_0x2a1; no reflected member covers 0x2a1.
// CONFIDENCE: HIGH
return (NativeRecoveredBits_0x2a1 & 0x01) != 0;
