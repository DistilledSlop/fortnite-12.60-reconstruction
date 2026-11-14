// Original body 0x144c23f80 (thunk 0x1451b9430): resolve the candidate pawn,
// return character vehicles directly, otherwise call its vehicle-state getter 0x1423254c0.
// CONFIDENCE: MEDIUM
static_assert(sizeof(AFortCharacterVehicle) > 0, "AFortCharacterVehicle must be complete for StaticClass check");
static_assert(sizeof(AFortPlayerStateAthena) > 0, "AFortPlayerStateAthena must be complete for GetCurrentPawn call");
AFortPlayerPawn* Candidate = nullptr;
if (PlayerState != nullptr)
{
    Candidate = PlayerState->GetCurrentPawn();
}
else
{
    Candidate = CachedPlayerPawn;
}
if (Candidate == nullptr)
{
    return nullptr;
}
if (Candidate->IsA(AFortCharacterVehicle::StaticClass()))
{
    return Candidate;
}
AFortAthenaVehicle* Vehicle = Candidate->NativeGetAthenaVehicle1423254c0();
return Vehicle;
