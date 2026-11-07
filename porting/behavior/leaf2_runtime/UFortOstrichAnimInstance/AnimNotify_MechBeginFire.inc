// Behaviour snippet: UFortOstrichAnimInstance::AnimNotify_MechBeginFire (body only; spliced into the generated member function).
// Original body address: 0x1418338b0 (thunk 0x142a0b0c0)
// Offset evidence: Byte at +0x438 bit 3 (mask 0x8) toggled: asm computes AL ^ 8. Member bIsMechBeginFired : 1 (bool_bits offset 0x438 mask 8).
// CONFIDENCE: MEDIUM

bIsMechBeginFired = !bIsMechBeginFired;
