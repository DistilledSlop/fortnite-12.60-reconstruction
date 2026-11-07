// Original body address: 0x1418c9bc0 (thunk 0x1427cfd80)
// Offset evidence: +0x114c byte is an unreflected gap (member_field NativeRecoveredByte_0x114c); nonzero returns true.
// Threshold: float at FortAntelopeVehicleConfigs (+0x11b8, reflected) + 0x4a4. Vectors are floats at +0xfb8..+0xfcc inside the
// unreflected LeanPositionBack struct, so they are read raw. Two squared-length tests against the threshold, each strictly greater.
// CONFIDENCE: HIGH
if (NativeRecoveredByte_0x114c != 0)
{
	return true;
}
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const uint8* Config = reinterpret_cast<const uint8*>(FortAntelopeVehicleConfigs);
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
