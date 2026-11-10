// CONFIDENCE: high (body from captured decompile; members from reflection; callee identified by address + argument pattern)
// Native body at 0x1429b23d0 (reflection exec entry): the 7-instruction prologue increments the FFrame+0x20 slot then tail-jumps to the real body at 0x1419fcba0 (asm: MOV RAX,[RDX+0x20]; SETNZ; ADD; MOV [RDX+0x20],R8; JMP 0x1419fcba0).
// Reflected members: DrivingPlayerController 0x1070, LandForceFeedbackHandle 0x10a0.
if (DrivingPlayerController)
{
	DrivingPlayerController->StopForceFeedback(2, LandForceFeedbackHandle);
}
