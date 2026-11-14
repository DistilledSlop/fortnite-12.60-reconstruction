// CONFIDENCE: HIGH
// Original 0x141f99860.
if (CurrentWorldRecord && bSucceeded)
{
    WorldManagerState = WMS_WorldQueryComplete;
    NativeWorldQueryResult_0x848.ExecuteIfBound(true, CurrentWorldRecord->WUID);
    return;
}
UE_LOG(LogFortWorld, Warning, TEXT("Invalid world record when transition to world query complete state!"));
WorldManagerState = WMS_Failed;
CurrentWorldRecord = nullptr;
const FString EmptyWUID;
NativeWorldQueryResult_0x848.ExecuteIfBound(false, EmptyWUID);
