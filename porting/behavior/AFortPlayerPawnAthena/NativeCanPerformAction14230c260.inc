// CONFIDENCE: HIGH. Complete original body; class lifecycle acceptance remains separate.
const bool bBaseAllowed = AFortPlayerPawn::NativeCanPerformAction14230c260(ActionTag);
// The original requires World and evaluates its GameState even after a true base result.
if (AFortGameStateAthena* State = Cast<AFortGameStateAthena>(GetWorld()->GetGameState()))
{
    const bool bStateAllowed = State->NativeQueryActionPermission1419afdf0(ActionTag);
    return bStateAllowed | bBaseAllowed;
}
return bBaseAllowed;
