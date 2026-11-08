// Original body: 0x144d79230 (thunk 0x1452044c0). Offset evidence: ADD RCX,0x1758 selects the reflected ButtonPadding struct (+0x1758). CALL 0x141f894c0 = NativeGetMarginBySize141f894c0(Base, SizeIndex) returns FMargin*; the 16 bytes at that pointer are copied to the hidden return slot (RDX, saved in RBX) and returned.
// CONFIDENCE: HIGH
FMargin* ResolvedMargin = UFortButtonStyle::NativeGetMarginBySize141f894c0(const_cast<uint8*>(reinterpret_cast<const uint8*>(&ButtonPadding)), static_cast<int32>(Size));
return *ResolvedMargin;
