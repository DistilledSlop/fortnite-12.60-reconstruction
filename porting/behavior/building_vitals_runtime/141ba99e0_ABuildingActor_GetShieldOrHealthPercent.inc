// Original address: 0x141ba99e0.
// CONFIDENCE: HIGH
// Direct callee; see building_vitals_runtime/README.md.
const float ShieldPercent = GetShieldPercent();
if (ShieldPercent > 0.0f)
{
	return ShieldPercent;
}
return GetHealthPercent();
