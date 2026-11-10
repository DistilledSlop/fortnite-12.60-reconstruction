// CONFIDENCE: MEDIUM. Exec thunk 0x14292bf70 reads Rescuer (object) and SurvivorName (FString) from the frame and forwards both to HandleSurvivorRescued (0x141ea3fe0, deps AFortGameStateZone__HandleSurvivorRescued).
// Original address: 0x14292bf70 (exec). Helpers: HandleSurvivorRescued (0x141ea3fe0); FMemory_Free of the temporary FString is engine cleanup.
HandleSurvivorRescued(Rescuer, SurvivorName);
