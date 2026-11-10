// CONFIDENCE: HIGH for control flow; MEDIUM for field meanings and the enum type name. Original addresses: exec 0x142a7bd90; helper 0x14231a050 (inlined here).
// Gender source: AttemptGetPlayerStateForPawn(this) (0x141ee5f70). A PlayerState supplies its +0x5d8 object; otherwise, if Hero (+0x1818) is set, HACK_CustomPRIComponent (+0x1828) is the source; otherwise the pawn-local byte at +0x1161 is returned. The source object's +0x28 field is a pointer to the stored gender byte.
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
// Enum type comes from the reflected ReturnValue (type ref 0x142b241d0).
return static_cast<EFortCharacterGender>(GenderValue);
