// Original body address: 0x1450d04c0 (thunk 0x1452ddc50). Offset evidence: Animations is the reflected TArray<FFortSimpleWidgetAnimations> at object +0x130 (Num at +0x138), element stride 0x38. Match is Name == Name (ComparisonIndex +0, Number +4). The matched element's byte at +0x30 is an unreflected field that the body clears; it is written raw.
// CONFIDENCE: MEDIUM
for (FFortSimpleWidgetAnimations& Anim : Animations)
{
	if (Anim.Name == Name)
	{
		uint8* ElementRaw = reinterpret_cast<uint8*>(&Anim);
		*(ElementRaw + 0x30) = 0;
		return true;
	}
}
return false;
