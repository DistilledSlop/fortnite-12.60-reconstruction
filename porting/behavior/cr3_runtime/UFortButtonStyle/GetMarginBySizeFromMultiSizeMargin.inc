// Original body: 0x144d7a5d0 (thunk 0x145204ec0). Offset evidence: static function; RCX is the hidden return slot, RDX is the MultiSizeMargin reference, R8D is Size. CALL 0x141f894c0 gets Base = RDX (the struct at offset 0) and SizeIndex = Size; the 16 bytes at the returned FMargin* are copied to the return slot.
// CONFIDENCE: HIGH
FMargin* ResolvedMargin = UFortButtonStyle::NativeGetMarginBySize141f894c0(reinterpret_cast<uint8*>(&MultiSizeMargin), static_cast<int32>(Size));
return *ResolvedMargin;
