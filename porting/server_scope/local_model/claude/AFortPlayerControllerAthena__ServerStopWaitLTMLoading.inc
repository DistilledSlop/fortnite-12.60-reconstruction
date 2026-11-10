// CONFIDENCE: MEDIUM. Original exec 0x142a5ca80 (RPC thunk, no parameters). There is no validate function: the thunk calls the implementation directly (virtual slot 0x2790 -> 0x1419c83c0, inlined below).
if (LogLTMLoadingVerbosity > 4)
{
	// Log arguments beyond the player name are not recovered from the decompile (needs-sync and IsServer). Name source is the object at controller+0x18 (LOW).
	UE_LOG(LogFortLTM, Verbose, TEXT("[LTMLoading] - Player %s StopWaitLTMLoading, needs sync %s, IsServer %s"), *GetNameForLog_0x142ce9f90(Field_0x18));
}
LTMLoadingWaitFlag = false; // controller+0x2d69
if (Role != ROLE_Authority)
{
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		// Timer manager for the loading timer: World+0x320, or the object at World+0x188 +0xc0 when that object exists (LOW).
		FTimerManager& TimerManager = (World->Field_0x188 == nullptr) ? World->Field_0x320 : World->Field_0x188->Field_0xc0;
		if (TimerManager.IsTimerHandleLive(LTMLoadingTimerHandle)) // FUN_144621fb0; name inferred from the existing porting directory name, body not fully read
		{
			TimerManager.InternalClearTimer(LTMLoadingTimerHandle); // FTimerManager_InternalClearTimer
		}
		LTMLoadingTimerHandle = 0; // controller+0x2e90
	}
}
