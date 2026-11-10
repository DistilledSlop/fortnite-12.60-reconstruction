// CONFIDENCE: HIGH for control flow; MEDIUM for field meanings. Original addresses: exec 0x142a7c1a0; helper 0x14231c0e0 (inlined here).
// Same gender source resolution as GetCharacterGender: PlayerState +0x5d8, else Hero-gated HACK_CustomPRIComponent +0x1828, else the pawn byte +0x1161. Value 1 means female, value 2 means male.
uint8 GenderValue = NativeRecoveredByte_0x1161;
AFortPlayerStateAthena* PlayerState = UFortKismetLibrary::AttemptGetPlayerStateForPawn(this);
UObject* GenderSource = nullptr;
if (PlayerState)
{
    GenderSource = PlayerState->NativeRecoveredPtr_0x5d8;
}
else if (Hero)
{
    GenderSource = HACK_CustomPRIComponent;
}
if (GenderSource)
{
    GenderValue = *GenderSource->NativeRecoveredGenderPtr_0x28;
}
IsFemale = (GenderValue == 1);
IsMale = (GenderValue == 2);
