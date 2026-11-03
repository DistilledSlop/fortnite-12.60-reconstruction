// Original address: 0x141b9ef10.
// CONFIDENCE: HIGH
// Virtual slot 0x7d0; see building_vitals_runtime/README.md.
const float MaxHealthValue = GetMaxHealth();
if (MaxHealthValue > 0.0f)
{
	const float Ratio = GetHealth() / MaxHealthValue;
	if (Ratio >= 0.0f)
	{
		return (Ratio < 1.0f) ? Ratio : 1.0f;
	}
}
return 0.0f;
