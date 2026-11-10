// CONFIDENCE: MEDIUM
// Original address: 0x142756780 (inline body, not a thunk). Stat scope push/pop (FUN_142cd3770) is diagnostic only and omitted.
// Bit ownership in the flag bytes is taken from field-offset names: 0x6ac bit 0x10 (bRegisterWithStructuralGrid) and 0x6a9 bit 0x4 (bNoPawnCollision); both unverified.
if (bRegisterWithStructuralGrid && !bNoPawnCollision)
{
    // Virtual +0x150 is UObject::GetWorld (0x143f793a0).
    auto World = GetWorld();
    // Structural grid for the world, or null (FUN_141c77490).
    if (auto Grid = GetStructuralGridForWorld(World))
    {
        // Marks the building and its connected neighbours (FUN_141c82b90).
        MarkConnectedBuildingsForIntegrityCheck(Grid, this);
    }
}
