// Original body address: 0x142681e40
// Offset evidence: MOVSS [RCX+0xf3c],XMM1 (float, position 1) -> NativeRecoveredFloat_0xf3c (unreflected gap; nearest reflected member is SelectionProperty at 0xf32). MOV byte [RCX+0xf36],R8B -> bIsPushPressed (reflected bool bit, bool_bits.json). MOV byte [RCX+0xf37],R9B -> bIsPullPressed (reflected bool bit). ABI positions match header_decl order.
// CONFIDENCE: HIGH
NativeRecoveredFloat_0xf3c = NewTargetTractorBeamOffset;
bIsPushPressed = bNewIsPushPressed;
bIsPullPressed = bNewIsPullPressed;
