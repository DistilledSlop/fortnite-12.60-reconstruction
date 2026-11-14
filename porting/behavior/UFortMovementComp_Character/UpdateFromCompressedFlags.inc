// CONFIDENCE: HIGH. Original1420f4c90; engine decode precedes Fort input and sprint is written last.
Super::UpdateFromCompressedFlags(Flags);
if (AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(CharacterOwner))
{
    const int32 ParachuteRequest = (Flags & 0x10) ? 2 : ((Flags >> 5) & 1);
    Pawn->NativeSetParachuteRequest14234bc70(ParachuteRequest);
    const bool bNewTargeting = (Flags & 0x08) != 0;
    if (Pawn->bIsTargeting != bNewTargeting)
    {
        Pawn->NativeSetTargeting14234f020(bNewTargeting);
    }
    Pawn->NativeSprintState1062Bit0 = (Flags & 0x40) != 0;
}
