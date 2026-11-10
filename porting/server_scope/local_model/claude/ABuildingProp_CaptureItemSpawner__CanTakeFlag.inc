// CONFIDENCE: MEDIUM. The flag can be taken only on an Athena game state, and only when that game state's permission check accepts the interacting pawn.
// Original address: 0x14274adc0 (exec). Native body: 0x141c005c0.
UWorld* World = GetWorld();
if (World != nullptr && World->GameState != nullptr)
{
    if (AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(World->GameState))
    {
        return CanPawnTakeFlag(GameState, InteractingPawn);   // native 0x141989960
    }
}
return false;
