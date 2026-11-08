// Helper 0x140690880 (47 insns): RCX = bit array, EDX = bits to reserve. Layout from asm: [+0x18] NumBits, [+0x1c] MaxBits,
// secondary word pointer at +0x10 after 16 inline bytes = TBitArray<TInlineAllocator<4>> (FDefaultBitArrayAllocator, 32 bytes).
// Asm: if (Number > MaxBits) { Words = (Number+31)/32; Words <= 4 ? 4 : CalculateSlackGrow(Words, prevWords) via QuantizeSize
// (0x142bfce10); MaxBits = Words*32; Realloc(NumBits) (0x140690310) } = TBitArray::Reserve. Caller 0x142561160 does
// Reserve(NumBits+1) followed by NumBits++ (TBitArray::Add).
// CONFIDENCE: HIGH
reinterpret_cast<TBitArray<FDefaultBitArrayAllocator>*>(BitArray)->Reserve(Number);
