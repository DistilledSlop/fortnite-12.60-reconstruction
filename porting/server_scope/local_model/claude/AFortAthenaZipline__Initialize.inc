// CONFIDENCE: MEDIUM. Original addresses: exec thunk 0x142842000, helper 0x14194dc60.
// Authority-only setup: the endpoints are stored, the zipline is flagged initialized, and the actor
// forces a network update.
if (Role != ROLE_Authority)
{
    return;
}
FlushNetDormancy();            // helper 0x143f72040 (named "NET_AActor_FlushNetDormancy")
StartPosition = NewStartPosition;   // offset 0x7d0
EndPosition = NewEndPosition;       // offset 0x7dc
bInitialized = true;                // offset 0x7e8
ForceNetUpdate();                   // vtable slot 0x5e8 -> 0x143f72550 (body residual)
