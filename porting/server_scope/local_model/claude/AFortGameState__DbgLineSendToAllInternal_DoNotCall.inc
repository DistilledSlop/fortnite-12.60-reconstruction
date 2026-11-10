// CONFIDENCE: HIGH. Exec thunk 0x14290d1c0 consumes Thickness, DepthPriority, LifeTime, bPersistentLines, Color, LineEnd, LineStart and forwards to virtual slot 0x728, native body 0x141e95de0. That body ignores all arguments and only reports release-mode misuse once per process.
// Original address: 0x14290d1c0 (exec). Native body: 0x141e95de0 (vtable slot 0x728 on AFortGameState).
// The shared once-flag is DAT_14812f404, declared in deps/AFortGameState__DbgDoNotCallWarnedFlag.json.
if (!GDbgDoNotCallWarnedFlag)
{
	if (LogFortnite.IsEnabled(ELogVerbosity::Warning))
	{
		UE_LOG(LogFortnite, Warning, TEXT("DbgLineSendToAllInternal_DoNotCall called in release mode."));
	}
	GDbgDoNotCallWarnedFlag = true;
}
