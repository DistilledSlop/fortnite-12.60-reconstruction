// CONFIDENCE: HIGH for control flow; MEDIUM for the gender store. Original addresses: _Implementation 0x14234a0f0 (read in full, 23 lines); helper 0x141ee5f70 (AttemptGetPlayerStateForPawn, read in full); SetGender helper 0x141da28f0 (read in full, inlined below).
// Gender target: PlayerState +0x5d8 when a PlayerState resolves; otherwise Hero-gated HACK_CustomPRIComponent (+0x1828). Gender 3 means random (male or female).
AFortPlayerStateAthena* PlayerState = UFortKismetLibrary::AttemptGetPlayerStateForPawn(this);
UObject* GenderTarget = nullptr;
if (PlayerState)
{
    GenderTarget = PlayerState->NativeRecoveredPtr_0x5d8;
}
else
{
    if (!Hero)
    {
        return;
    }
    GenderTarget = HACK_CustomPRIComponent;
}
if (GenderTarget)
{
    // 0x141da28f0: stores the byte through the target's +0x28 pointer. Gender 3 becomes 1 or 2 from rand() bit 0 (bit 0 set gives 1, clear gives 2).
    uint8 StoredGender = static_cast<uint8>(Gender);
    if (Gender == 3)
    {
        StoredGender = (rand() & 1) ? 1 : 2;
    }
    *GenderTarget->NativeRecoveredGenderPtr_0x28 = StoredGender;
}
