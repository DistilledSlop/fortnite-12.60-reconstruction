// CONFIDENCE: HIGH
// Original addresses: RPC thunk 0x14290cd10 (reads Center, Extent, Rotation, Color, bPersistentLines, LifeTime,
// DepthPriority, Thickness and forwards them to virtual slot 0x718, owner AFortGameState) -> _Implementation 0x141e95d40.
// The implementation takes no parameters and never reads the debug-box arguments, so they are not used here.
// Behaviour: warn once that the function was called in release mode.
if (!bDbgBoxWarningLogged)                                      // DAT_14812f404
{
	if (LogFortnite_Verbosity > 1)                              // DAT_1481348e0 > 1 (Warning level)
	{
		UE_LOG(LogFortnite, Warning, TEXT("DbgBoxSendToAllInternal_DoNotCall called in release mode."));
	}
	bDbgBoxWarningLogged = true;
}
