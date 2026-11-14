// CONFIDENCE: HIGH. Complete original body; class lifecycle acceptance remains separate.
// Original 0x141a127f0; cache the byte before any world or mutator callbacks.
const bool bOriginalAllow = bADSWhileNotOnGround;
UWorld* World = GetWorld();
if (!World)
{
    return bOriginalAllow;
}
AFortGameStateAthena* State = Cast<AFortGameStateAthena>(World->GetGameState());
if (!State)
{
    return bOriginalAllow;
}
AFortPlayerState* StateForQuery = Cast<AFortPlayerState>(GetPlayerState());
if (State->NativeEvaluateADSMutators141989680(StateForQuery))
{
    return true;
}
return bOriginalAllow;
