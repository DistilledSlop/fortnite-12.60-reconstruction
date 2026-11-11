// CONFIDENCE: HIGH - RPC validate (vslot 0xaa8) resolves to Stub_ReturnTrue so the ValidateFailed branch is unreachable; implementation 0x1426df570 read in full.
// Original addresses: exec 0x142b4fcf0; impl 0x1426df570 (vslot 0xab0)
AFortPawn* Pawn = Cast<AFortPawn>(Instigator);
if (Pawn == nullptr)
{
    return;
}
bIsMuzzleTraceNearWall = bIsNearWall;
Pawn->FlushNetDormancy();
FlushNetDormancy();
