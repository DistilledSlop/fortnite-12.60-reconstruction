// Original body address: 0x1450de300 (thunk 0x145232bd0)
// Offset evidence: no object member access. Static ABI: RCX=Expedition, XMM1=TotalPower. Calls UFortExpeditionItem::GetMinTargetPowerLevelToBegin (0x1424e6dc0) and GetTargetPowerLevel (0x1424e8c80) on Expedition.
// Clamp and lerp constants (1.0, 0.05) as in the decompile of the body. Min > TotalPower returns 0; ratio is clamped to [0,1]; result = (1 - clamp) * 0.05 + clamp.
// CONFIDENCE: HIGH
if (Expedition == nullptr)
{
	return 0.0f;
}
const float MinPower = Expedition->GetMinTargetPowerLevelToBegin();
if (MinPower > TotalPower)
{
	return 0.0f;
}
const float TargetPower = Expedition->GetTargetPowerLevel();
const float Ratio = (TotalPower - MinPower) / (TargetPower - MinPower);
float Clamped = 0.0f;
if (0.0f <= Ratio)
{
	Clamped = Ratio;
	if (1.0f <= Ratio)
	{
		Clamped = 1.0f;
	}
}
return (1.0f - Clamped) * 0.05f + Clamped;
