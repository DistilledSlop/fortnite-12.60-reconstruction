// CONFIDENCE: HIGH. Exec thunk 0x14290cf40 consumes Thickness, DepthPriority, LifeTime, bPersistentLines, Color, Rotation, Radius, HalfHeight, Center and forwards to virtual slot 0x720, whose native body is 0x141e95d90 (the reflected DbgCapsuleSendToAllInternal_DoNotCall itself). That body ignores all arguments and only reports release-mode misuse once per process.
// Original address: 0x14290cf40 (exec). Native body: 0x141e95d90 (vtable slot 0x720 on AFortGameState).
// The shared once-flag is DAT_14812f404, declared in deps/AFortGameState__DbgDoNotCallWarnedFlag.json.
if (!GDbgDoNotCallWarnedFlag)
{
	if (LogFortnite.IsEnabled(ELogVerbosity::Warning))
	{
		UE_LOG(LogFortnite, Warning, TEXT("DbgCapsuleSendToAllInternal_DoNotCall called in release mode."));
	}
	GDbgDoNotCallWarnedFlag = true;
}
