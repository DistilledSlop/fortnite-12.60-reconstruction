// Original address: 0x1428c8aa0.
// CONFIDENCE: high
// Reflected native (flags 0x4080401 = Net|NetServer|Native), no reflected params (SDK 23.20:
// Final|Native|Protected|BlueprintCallable, void, matches).
// (1) Standard FFrame::NextLocalVar guard-increment on the script frame (exec param 2):
//     value 0 stays 0, non-zero n becomes n+1.
// (2) Tail-jump (JMP, not CALL) to shared helper 0x141d73520: zero the first N int32
//     entries of the ResourcesGivenPerRound array — 5 int32s at this+0xc14 — where
//     N = [this+0xc44]. The helper skips entirely when N <= 0 (CMP/JLE) and re-reads
//     N from [this+0xc44] on each iteration; nothing in the loop writes [this+0xc44],
//     so a plain bound loop is behaviorally identical.
// NOTE: the original bounds this clear with [this+0xc44] — the same count field used by
// GetResourceGivenToWinnersPerRound (0x141d84dd0) and SetResourceGivenToWinnersPerRound
// (0x141d91030) — NOT the [this+0xc28] bound used by Get/SetResourceGivenPerRound
// (0x141d84db0/0x141d91000). This asymmetry is in the original and is preserved.
*(uint8**)((uint8*)&Stack + 0x20) += (*(uint8**)((uint8*)&Stack + 0x20) != nullptr);
for (int32 i = 0; i < NativeRecoveredInt32_0xc44; ++i)
{
	NativeRecoveredInt32_0xc14[i] = 0;
}
