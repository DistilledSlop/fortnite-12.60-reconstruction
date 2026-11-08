// Helper 0x142cf1050 (7 insns): XMM0 = seconds (callers 0x1409a6db0 pass a constant, 0x1443b8100 passes 0.0f in a spin loop
// on a flag byte). MULSS by 1000.0f (0x1459a872c = 0x447a0000), CVTTSS2SI; ms == 0 -> tail jump IAT 0x1458fc5b8 =
// KERNEL32!SwitchToThread, else Sleep(ms) via IAT 0x1458fcab0 = KERNEL32!Sleep. That is FPlatformProcess::SleepNoStats.
// CONFIDENCE: HIGH
FPlatformProcess::SleepNoStats(Seconds);
