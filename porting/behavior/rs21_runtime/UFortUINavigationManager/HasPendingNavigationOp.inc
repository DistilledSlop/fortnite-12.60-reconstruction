// Original body 0x144dc58d0 (thunk 0x145314e90). Offset evidence: bool +0x130 gap member (NativeRecoveredBool_0x130); NavigationRequests TArray Data +0x138 / Num +0x140 (reflected); FFortUINavigationRequest Operations TArray at element +0 (Data) and +8 (Num); FFortUINavigationOperation Operation byte at +0.
// CONFIDENCE: HIGH
if (!NativeRecoveredBool_0x130)
{
    return false;
}
if (NavigationRequests.Num() <= 0 || NavigationRequests[0].Operations.Num() <= 0)
{
    return false;
}
return NavigationRequests[0].Operations[0].Operation == NavigationOp;
