// CONFIDENCE: LOW - exec 0x1427a4710 builds a temporary FSharedRepMovement (init 0x1422b7e30) and calls vtable slot 0xe28 -> 0x1422c5910.
// Original addresses: 0x1427a4710 (exec thunk) ; 0x1422c5910 (virtual body, vtable slot 0xe28)
// Structural port of the body. Field names are proposed (no reflected names); helpers are declared deps in work/claude/out/deps.
UWorld* World = GetWorld();
if (IsSharedReplicationSuppressed(World))
{
    return;
}
if (Role != ROLE_SimulatedProxy)
{
    return;
}
if (SharedRepMovementIsNewer(SharedRepMovement.SequenceNumber, SharedSequenceNumber))
{
    SharedSequenceNumber = SharedRepMovement.SequenceNumber;
    if (bSharedMovementFlagA != SharedRepMovement.bFlagA)
    {
        bSharedMovementFlagA = SharedRepMovement.bFlagA;
        if (UFortMovementComponent* MoveComp = MovementComponent)
        {
            MoveComp->MarkSharedMovementStateDirty();   // proposed; original sets bits 0x80 and 0x40 of a byte at +0x38b
        }
    }
    if (bSharedMovementFlagB != SharedRepMovement.bFlagB)
    {
        bSharedMovementFlagB = SharedRepMovement.bFlagB;
        MarkSharedMovementDirty();   // vtable slot 0xdf8
    }
    if (SharedMovementApplyPredicate(SharedMovementState, SharedRepMovement))
    {
        ApplySharedRepMovement(SharedRepMovement);   // FUN_143f85020
        bApplyingSharedMovement = true;
        ReceiveSharedMovementUpdate();   // vtable slot 0x420
        bApplyingSharedMovement = false;
    }
}
AFortGameStateAthena* GameState = World != nullptr ? Cast<AFortGameStateAthena>(World->GameState) : nullptr;
if (GameState == nullptr)
{
    return;
}
AActor* NewSharedBase = nullptr;
if (SharedRepMovement.SharedBaseIndex != -1 && SharedRepMovement.SharedBaseIndex < GameState->SharedMovementBases.Num())
{
    NewSharedBase = GameState->SharedMovementBases[SharedRepMovement.SharedBaseIndex].Get();
}
if (SharedBase != NewSharedBase || bSharedBaseFlagC != SharedRepMovement.bFlagC || bSharedBaseFlagD != SharedRepMovement.bFlagD)
{
    SharedBase = NewSharedBase;
    bSharedBaseFlagC = SharedRepMovement.bFlagC;
    bSharedBaseFlagD = SharedRepMovement.bFlagD;
    if (UBoundMovementComponent* Bound = BoundMovementComponent)
    {
        if (Bound->IsA(UBoundMovementComponent::StaticClass()))
        {
            Bound->SetSharedBaseState(SharedRepMovement.bFlagD, SharedBaseFlagCounter, SharedBase, SharedBaseFlagCounter2);   // proposed; original writes +0x14d8..+0x14e4 then calls 0x141609170
            UpdateBoundMovementComponentBase(Bound);   // FUN_141609170
        }
    }
}
