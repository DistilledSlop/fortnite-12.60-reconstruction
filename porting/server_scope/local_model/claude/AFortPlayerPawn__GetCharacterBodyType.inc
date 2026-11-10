// CONFIDENCE: MEDIUM
// Original address: 0x142a7bcd0 (exec thunk, no parameters, returns uint8). Calls FUN_142319f60(this).
// Captures: captures/13799d83931e48b19e2b5ccf2a1528b5/0x142a7bcd0.c ; FUN_142319f60 captures/dcc736e51fb54d23a072dec1fe63306e/0x142319f60.c ; FUN_141ee5f70 captures (depth-two, read in full, see deps).
APlayerState* PS = GetFortPlayerStateForPawn(this);   // FUN_141ee5f70, read in full (see deps/AFortPlayerPawn__GetCharacterBodyType__FUN_141ee5f70.json)
UObject* BodySource = nullptr;
if (PS == nullptr)
{
	if (Hero == nullptr)
	{
		return 1;
	}
	BodySource = HACK_CustomPRIComponent;   // +0x1828
}
else
{
	BodySource = PS->BodyTypeSource;   // PlayerState +0x5d8, name not recovered
}
if (BodySource == nullptr)
{
	return 1;
}
return BodySource->BodyType;   // byte at offset 0x30 of the source, name not recovered
