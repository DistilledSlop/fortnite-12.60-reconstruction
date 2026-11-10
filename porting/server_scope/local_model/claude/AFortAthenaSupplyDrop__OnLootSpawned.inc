// CONFIDENCE: LOW. Proposed lift of candidate 0x1419237d0 reached from exec thunk
// 0x1428347d0 through the protected gateway. Emulation confirms receiver preservation
// for sampled direct and no-module-match paths; no live supply-drop invocation observed.
// Registry accessor and method names are descriptive placeholders, as in the related
// CaptureItemSpawner draft. The original body does not clear SpecialActorID.
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
