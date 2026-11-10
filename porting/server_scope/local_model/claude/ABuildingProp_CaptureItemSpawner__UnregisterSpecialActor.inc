// CONFIDENCE: MEDIUM. Authority-only. When this spawner holds a special-actor id, removes that entry from the game state's registry and clears the id.
// Original address: 0x14274c0b0.
if (Role != ROLE_Authority || SpecialActorID.IsNone())
{
    return;
}
UWorld* World = GetWorld();
if (World == nullptr)
{
    return;
}
AFortGameStateAthena* GameState = GetFortGameStateAthena(World);
if (GameState == nullptr || GameState->SpecialActorRegistry == nullptr)
{
    return;
}
GameState->SpecialActorRegistry->RemoveByKey(SpecialActorID);
SpecialActorID = NAME_None;
