// dep AFortGameStateZone__ResetGameplayModifierDirtyBits original 0x14072a990
// CONFIDENCE: MEDIUM. Original 0x14072a990. Field names are offsets into the list: +0x8 NumBits, +0x34 NumBitsPrev, +0x28 BitCount, +0x30 IndexA, +0x48 MaskCount, +0x10/+0x20 bit storage, +0x38/+0x40 mask storage.
if (NumBits != NumBitsPrev)            // +0x8 != +0x34
{
	NumBits = 0;                       // +0x8
	IndexA = INDEX_NONE;               // +0x30
	NumBitsPrev = 0;                   // +0x34
	ZeroBitWords((BitCount + 31) / 32);   // memset 0 over the bit words (+0x28 bits)
	BitCount = 0;                      // +0x28
	if (MaskCount > 0)                 // +0x48
	{
		SetMaskWordsToOnes(MaskCount); // memset 0xff over the mask words
	}
}
