// Original address: 0x141ba9a70.
// CONFIDENCE: HIGH
// Virtual slot 0x7c8; see building_vitals_runtime/README.md.
const float MaxShieldValue = GetMaxShield();
if (MaxShieldValue > 0.0f)
{
	return GetShield() / MaxShieldValue;
}
return 0.0f;
