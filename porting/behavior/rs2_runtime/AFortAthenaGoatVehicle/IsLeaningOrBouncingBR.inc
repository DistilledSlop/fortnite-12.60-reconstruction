// Original body address: 0x1418f1430 (thunk 0x1427db490)
// Offset evidence: +0x10c4 byte is an unreflected gap (member_field NativeRecoveredByte_0x10c4); nonzero returns true.
// Threshold: float at FortGoatVehicleConfigs (+0x1118, reflected) + 0x4a4. Vector floats at +0xfe0..+0xff4 are raw reads inside
// the unreflected LeanPositionBR struct. Two squared-length tests against the threshold, each strictly greater.
// CONFIDENCE: HIGH
if (NativeRecoveredByte_0x10c4 != 0)
{
	return true;
}
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const uint8* Config = reinterpret_cast<const uint8*>(FortGoatVehicleConfigs);
const float Limit = *reinterpret_cast<const float*>(Config + 0x4a4);
const float A1 = *reinterpret_cast<const float*>(Raw + 0xfe0);
const float B1 = *reinterpret_cast<const float*>(Raw + 0xfe4);
const float C1 = *reinterpret_cast<const float*>(Raw + 0xfe8);
const float A2 = *reinterpret_cast<const float*>(Raw + 0xfec);
const float B2 = *reinterpret_cast<const float*>(Raw + 0xff0);
const float C2 = *reinterpret_cast<const float*>(Raw + 0xff4);
if (((A1 * A1 + B1 * B1) + C1 * C1) > Limit)
{
	return true;
}
if (((A2 * A2 + B2 * B2) + C2 * C2) > Limit)
{
	return true;
}
return false;
