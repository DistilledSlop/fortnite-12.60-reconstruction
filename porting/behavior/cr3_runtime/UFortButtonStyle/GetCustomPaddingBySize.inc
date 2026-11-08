// Original body: 0x144d79690 (thunk 0x145204880). Offset evidence: ADD RCX,0x1998 selects the reflected CustomPadding struct (+0x1998). CALL 0x141f894c0 = NativeGetMarginBySize141f894c0(Base, SizeIndex) returns FMargin*; the 16 bytes at that pointer are copied to the hidden return slot (RDX, saved in RBX) and returned.
// CONFIDENCE: HIGH
FMargin* ResolvedMargin = UFortButtonStyle::NativeGetMarginBySize141f894c0(const_cast<uint8*>(reinterpret_cast<const uint8*>(&CustomPadding)), static_cast<int32>(Size));
return *ResolvedMargin;
