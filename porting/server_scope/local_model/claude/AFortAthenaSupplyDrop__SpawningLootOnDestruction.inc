// CONFIDENCE: MEDIUM. Original addresses: exec thunk 0x1428351f0, helper 0x141922280.
// Credits the instigator's player state with the supply-drop gameplay-tag score events.
if (EventInstigator != nullptr)
{
    APlayerState* PlayerState = EventInstigator->PlayerState;   // AController::PlayerState (offset 0x220)
    if (PlayerState != nullptr && PlayerState->IsA(AFortPlayerStateAthena::StaticClass()))
    {
        // Helper 0x141a4cf10 (dep AFortAthenaSupplyDrop__RecordTaggedScoreEvents, partially ported).
        AFortAthenaSupplyDrop__RecordTaggedScoreEvents(PlayerState, StaticGameplayTags);
    }
}
