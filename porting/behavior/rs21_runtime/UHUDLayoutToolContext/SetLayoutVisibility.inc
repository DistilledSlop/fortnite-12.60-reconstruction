// Original body 0x144d63f80 (thunk 0x14532bff0). Offset evidence: OnHUDLayoutModeChangeDelegate at +0xa8 (reflected). Windows x64 ABI: RDX=bShowCombat, R8=bShowBuild, R9=bShowEdit, stack arg=bShowCreative; the four bytes are packed in that order and passed to delegate broadcast 0x1406a9dc0 (FOnHUDLayoutModeChange FourParams, same order).
// CONFIDENCE: HIGH
OnHUDLayoutModeChangeDelegate.Broadcast(bShowCombat, bShowBuild, bShowEdit, bShowCreative);
