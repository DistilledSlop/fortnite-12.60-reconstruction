// Body 0x142098de0 (thunk 0x1429c2910). Offset evidence: [this+0xaa8] is MinigameStarter (reflected AFortPlayerState*). The body null-checks it and tests GUObjectArray item flag bit 29 (PendingKill) through its InternalIndex, which is UObject::IsPendingKill.
// CONFIDENCE: HIGH
static_assert(sizeof(AFortPlayerState) > 0, "AFortPlayerState must be complete for MinigameStarter->IsPendingKill()");
return MinigameStarter != nullptr && !MinigameStarter->IsPendingKill();
