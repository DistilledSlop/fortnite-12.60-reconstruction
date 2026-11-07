// Original body address: 0x144f1f850 (thunk 0x1451f25b0, group B, 12 instructions).
// Offset evidence: this+0x2a4 byte is the reflected bitfield bUpgradeOptionExists (mask 1, bool_bits.json). The asm returns the whole byte.
// this+0x2e0 is the reflected TArray RespecOptions; its Num() is the dword at +0x2e8 (array Num at O+8 per the offset rules).
// CONFIDENCE: MEDIUM
const uint8 OptionTypeValue = static_cast<uint8>(OptionType);
if (OptionTypeValue == 0)
{
	return bUpgradeOptionExists;
}
if (OptionTypeValue == 1)
{
	return RespecOptions.Num() > 0;
}
return false;
