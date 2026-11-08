// Helper 0x141f8e580 (UFortKismetLibrary), callers 0x141ef7ee0 (DL=0 tail-jump) and 0x1424b8330 (DL=0 before call).
// Asm evidence: true if int32 at [+0x18] > 0, or int32 at [+0x8] > 0, or (DL != 0 and int32 at [+0x28] > 0); else false.
// The [+0x8]/[+0x18]/[+0x28] dwords are the Num fields of TArrays at offsets 0x0/0x10/0x20.
// CONFIDENCE: HIGH
const uint8* RawRewards_hq10 = reinterpret_cast<const uint8*>(Rewards);
if (*reinterpret_cast<const int32*>(RawRewards_hq10 + 0x18) > 0)
{
	return true;
}
if (*reinterpret_cast<const int32*>(RawRewards_hq10 + 0x8) > 0)
{
	return true;
}
if (!bIncludeExtended)
{
	return false;
}
return *reinterpret_cast<const int32*>(RawRewards_hq10 + 0x28) > 0;
