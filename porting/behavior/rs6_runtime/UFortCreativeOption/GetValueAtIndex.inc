// Behaviour snippet: UFortCreativeOption::GetValueAtIndex (body only; spliced into the generated member function).
// Original address: 0x141d85c50
// Offset evidence: Options TArray at +0x100 (Data) and +0x108 (Num). Element stride 0x20 with int32 at element +0x18, which is FCreativeOptionData::Value. Index below 0 or not below Num returns -1.
// CONFIDENCE: HIGH
if (Index < 0 || Index >= Options.Num())
{
	return -1;
}
return Options[Index].Value;
