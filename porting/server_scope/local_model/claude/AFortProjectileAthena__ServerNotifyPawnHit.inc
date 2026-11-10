// CONFIDENCE: MEDIUM. Exec thunk: reads the FHitResult struct from the script frame, runs the RPC _Validate (vslot 0x6f8 -> 0x1406801d0, STUB Stub_ReturnTrue, so the validate-failed branch is unreachable), then _Implementation (vslot 0x700 -> 0x141a59110).
// Original address: 0x142aa6590 (exec). Validate: 0x1406801d0 (STUB). Implementation: 0x141a59110 (see deps AFortProjectileAthena__ServerNotifyPawnHit__Implementation.json).
FHitResult Hit = HitFromFrame;   // struct read in thunk order; default-initialised fields copied as read
if (!ServerNotifyPawnHit_Validate(Hit))
{
    RPC_ValidateFailed(TEXT("ServerNotifyPawnHit_Validate"));
}
else
{
    ServerNotifyPawnHit_Implementation(Hit);
}
