// Helper 0x1423c0800: a single `JMP 0x142810956`. thunk-table/final3.json resolves the chain
// 0x142374290 -> 0x1423c0800 -> 0x142810956 as the real body of AFortPlayerController::ClientUpdateRichPresence
// (params by C++ order: RichPresenceChange enum byte, bImportant bool).
// Callers (asm): RCX = player controller, RDX = 0 (enum value), R8B = 1 or 0 (bImportant); result unused.
const ERichPresenceStateChange RequestedChange = static_cast<ERichPresenceStateChange>(PresenceChange);
if (RichPresenceController != nullptr)
{
	RichPresenceController->ClientUpdateRichPresence(RequestedChange, bPresenceImportant);
}
// CONFIDENCE: HIGH
