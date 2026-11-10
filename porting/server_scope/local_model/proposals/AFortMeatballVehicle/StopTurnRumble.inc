// CONFIDENCE: high (body from captured decompile; members from reflection; callee identified by address + argument pattern)
// Native body at 0x1429b23f0 (reflection exec entry): 7-instruction prologue increments the FFrame+0x20 slot then tail-jumps to the real body at 0x1419fcf90 (asm: MOV RAX,[RDX+0x20]; SETNZ; ADD; MOV [RDX+0x20],R8; JMP 0x1419fcf90).
// Reflected members: DrivingPlayerController 0x1070, TurnRumbleIntensity 0x10a8 (u32), TurnForceFeedbackHandle 0x10b0, bTurnRumbleActive 0x10bf (bool bit).
if (bTurnRumbleActive)
{
	TurnRumbleIntensity = 0;
	if (DrivingPlayerController)
	{
		DrivingPlayerController->StopForceFeedback(2, TurnForceFeedbackHandle);
	}
	bTurnRumbleActive = false;
}
