// Original body 0x1419db530 (thunk 0x142a58d20) for AFortPlayerControllerAthena::HandlePlayersLeftChanged.
// Offset evidence: no memory access; ABI: RCX = this unchanged, EDX = 0 (PresenceChange), R8B = 1 (bPresenceImportant); body is a JMP to 0x1423c0800.
// CONFIDENCE: HIGH
static_assert(sizeof(AFortGameStateZone) > 0, "AFortGameStateZone must be complete");
AFortGameStateZone::NativeClientUpdateRichPresence1423c0800(this, 0, true);
