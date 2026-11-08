// Native body 0x1423ac440 (thunk 0x142a4d0b0); offset evidence: [RCX+0x1eec] = AFortPlayerController::AntiAddictionPlayTimeMultiplier (reflected Float at 0x1eec, lookup_offset.py).
// Asm: XORPS/COMISS XMM1,0 + JNC gate (rate >= 0 and ordered); MINSS XMM1,[.rdata 0x1459a86f8 = 1.0f]; store dword; else store 0.0f.
// CONFIDENCE: HIGH
if (NewPlayTimeRewardRate >= 0.0f)
{
	AntiAddictionPlayTimeMultiplier = (NewPlayTimeRewardRate < 1.0f) ? NewPlayTimeRewardRate : 1.0f;
}
else
{
	AntiAddictionPlayTimeMultiplier = 0.0f;
}
