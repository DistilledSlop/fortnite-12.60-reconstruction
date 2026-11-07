// Original body address: 0x141ab1dc0 (thunk 0x1427e1710)
// Offset evidence: this+0xf0 = AActor::Role (Byte) compared with 3 (ROLE_Authority). this+0x260 = CachedGameState (AFortAthenaMutator, Object).
// GameState+0x1f00 = MutatorEventData.EventId (FGameplayMutatorEventData offset 0), written with 1 and no null check (as in asm).
// SafeZoneInterface (RDX): ObjectPointer null test, then getter 0x14290f690 = AFortGameStateAthena::StaticClass(); the object's class
// depth/superstruct check is IsA(X). GamePhaseStep (R8B) compared with 4.
// CONFIDENCE: MEDIUM
if (static_cast<uint8>(Role) != 3)
{
    return;
}
UObject* SafeZoneObject = SafeZoneInterface.GetObject();
if (SafeZoneObject == nullptr || !SafeZoneObject->IsA(AFortGameStateAthena::StaticClass()))
{
    return;
}
if (static_cast<uint8>(GamePhaseStep) != 4)
{
    return;
}
CachedGameState->MutatorEventData.EventId = 1;
