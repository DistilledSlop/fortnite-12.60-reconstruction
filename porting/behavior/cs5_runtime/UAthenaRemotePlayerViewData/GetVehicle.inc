// Original body 0x144c23f80 (thunk 0x1451b9430): candidate = [this+0x1d0] PlayerState ? PlayerState->GetCurrentPawn() (0x1419b6d20) : [this+0x228] CachedPlayerPawn; null -> return null; IsA(AFortCharacterVehicle) -> return candidate; else tail-jump 0x1423254c0 = NativeGetVehicleFromViewData1423254c0(candidate)
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
return reinterpret_cast<AActor*>(UAthenaRemotePlayerViewData::NativeGetVehicleFromViewData1423254c0(reinterpret_cast<UAthenaRemotePlayerViewData*>(Candidate)));
