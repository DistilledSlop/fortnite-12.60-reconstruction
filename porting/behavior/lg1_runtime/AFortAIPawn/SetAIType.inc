// Original body address: 0x1422ec0c0
// Evidence: MOV byte [RCX+0x18e1],DL stores AIType (reflected Enum, 1 byte). OR/AND dword [RCX+0x18dc],0x800 is byte +0x18dd mask 0x08 = bCanSleep (declared bitfield, bool_bits.json).
// DL in {1,2,3,5} clears bCanSleep; DL in {0,4} and DL >= 6 set it.
// Verbosity-gated diagnostic log at call 0x142c54b70 elided (no state change).
// CONFIDENCE: MEDIUM
AIType = InAIType;
const uint8 Value = static_cast<uint8>(InAIType);
bCanSleep = !(Value == 1 || Value == 2 || Value == 3 || Value == 5);
