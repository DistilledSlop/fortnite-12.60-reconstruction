// Original body address: 0x1418f14b0 (thunk 0x1427db4c0)
// Offset evidence: +0x10b8 byte is an unreflected gap (member_field NativeRecoveredByte_0x10b8); nonzero returns true.
// Threshold: float at FortGoatVehicleConfigs (+0x1118, reflected) + 0x4a4. Vector floats at +0xfb8..+0xfcc are raw reads inside
// the unreflected LeanPositionFR struct. Two squared-length tests against the threshold, each strictly greater.
// CONFIDENCE: HIGH
if (NativeRecoveredByte_0x10b8 != 0)
{
	return true;
}
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const uint8* Config = reinterpret_cast<const uint8*>(FortGoatVehicleConfigs);
const float Limit = *reinterpret_cast<const float*>(Config + 0x4a4);
const float A1 = *reinterpret_cast<const float*>(Raw + 0xfb8);
const float B1 = *reinterpret_cast<const float*>(Raw + 0xfbc);
const float C1 = *reinterpret_cast<const float*>(Raw + 0xfc0);
const float A2 = *reinterpret_cast<const float*>(Raw + 0xfc4);
const float B2 = *reinterpret_cast<const float*>(Raw + 0xfc8);
const float C2 = *reinterpret_cast<const float*>(Raw + 0xfcc);
if (((A1 * A1 + B1 * B1) + C1 * C1) > Limit)
{
	return true;
}
if (((A2 * A2 + B2 * B2) + C2 * C2) > Limit)
{
	return true;
}
return false;
