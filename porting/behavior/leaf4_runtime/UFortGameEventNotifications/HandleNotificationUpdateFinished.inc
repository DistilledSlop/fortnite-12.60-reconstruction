// Behaviour snippet: UFortGameEventNotifications::HandleNotificationUpdateFinished (body only; spliced into the generated member function).
// Original address: 0x140743d00 (thunk 0x145249cc0).
// asm: MOVZX EAX,byte [+0x38]; RET (the read lands inside the reflected bIsEnabledDelegate span +0x30..+0x3f). Void function; the load into RAX has no store and no observable effect.
// CONFIDENCE: HIGH
return;
