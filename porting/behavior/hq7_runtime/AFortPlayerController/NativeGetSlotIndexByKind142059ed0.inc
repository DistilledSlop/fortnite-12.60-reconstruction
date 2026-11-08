// Helper 0x142059ed0 (AFortPlayerController), asm porting/generated/native102-impl/0x142059ed0.asm.
// Returns int32 at Controller+X chosen by Kind: Kind>=3 -> -1; Kind 0 -> +0x220; Kind 1 -> +0x2b0; Kind 2 -> +0x340.
// Offset evidence (lookup_offset.py): +0x220 is AController::PlayerState (Object), +0x2b0 is APlayerController::PlayerCameraManager (Object),
// +0x340 is APlayerController::PlayerInput (Object). The asm reads them as int32 slot indices, so raw access with the asm width is used.
// Callers (ActivateQuickbarSlot 0x141d70410, HandleMoveToolInteractionEnded 0x141d86ad0) compare the result against a slot index.
// CONFIDENCE: HIGH
if (Kind >= 3)
{
    return -1;
}
uint32 MemberOffset = 0x220;
if (Kind == 1)
{
    MemberOffset = 0x2b0;
}
else if (Kind == 2)
{
    MemberOffset = 0x340;
}
return *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(Controller) + MemberOffset);
