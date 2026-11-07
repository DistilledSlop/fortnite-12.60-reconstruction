// Original body 0x144de4420 (thunk 0x145315d30). Offset evidence: bool +0x130 gap member (NativeRecoveredBool_0x130); NavigationRequests Data +0x138 / Num +0x140 (reflected); Operations Data/Num at element +0/+8. The body copies 0x30 bytes (three MOVUPS) from Operations[0] into the out reference, the size of FFortUINavigationOperation.
// CONFIDENCE: MEDIUM
if (!NativeRecoveredBool_0x130)
{
    return false;
}
if (NavigationRequests.Num() <= 0 || NavigationRequests[0].Operations.Num() <= 0)
{
    return false;
}
NavigationOp = NavigationRequests[0].Operations[0];
return true;
