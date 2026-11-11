// CONFIDENCE: LOW
// Native 0x14296bf00 -> 0x141eefff0: resets the output and fills it with the world's AFortPlayerPawn actors through a
// filtered actor iterator (filter flags 5). Pending-kill pawns are skipped and only player-controlled pawns are kept.
OutFortPlayerPawns.Reset();
const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
if (!World)
{
    return;
}
for (TActorIterator<AFortPlayerPawn> It(World); It; ++It)
{
    AFortPlayerPawn* Pawn = *It;
    if (!IsValid(Pawn) || Pawn->IsPendingKill())
    {
        continue;
    }
    if (!Pawn->IsPlayerControlled())
    {
        continue;
    }
    OutFortPlayerPawns.Add(Pawn);
}
