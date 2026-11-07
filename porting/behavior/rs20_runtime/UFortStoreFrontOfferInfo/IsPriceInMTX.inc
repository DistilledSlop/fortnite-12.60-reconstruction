// Original body address: 0x1450b9870 (thunk 0x1452f7c70). Offset evidence: object +0x28 is a sub-object pointer below this class's own region (own min 0x40), read raw. Pointee fields are unreflected: byte +0x60 (==1 selects the single-value path, tested against byte +0x84 == 1), int32 +0x70 (count), pointer +0x68 (entries, 0x48 stride, byte +0 == 1 test).
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const uint8* Info = *reinterpret_cast<const uint8* const*>(Raw + 0x28);
if (*(Info + 0x60) == 1)
{
	return *(Info + 0x84) == 1;
}
if (PriceIndex < 0 || PriceIndex >= *reinterpret_cast<const int32*>(Info + 0x70))
{
	return false;
}
const uint8* Entries = *reinterpret_cast<const uint8* const*>(Info + 0x68);
return *(Entries + static_cast<int64>(PriceIndex) * 0x48) == 1;
