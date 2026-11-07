// Original body address: 0x144d666e0 (thunk 0x14521d740)
// Offset evidence: +0x510 is TargetPortal (reflected Object, AFortAthenaCreativePortal*). The asm stores Portal only when it is non-null (TEST RDX,RDX; JZ).
// Verbosity-gated diagnostic logs at 0x142c54b70 are elided on both branches (gate byte 0x14838b9f0 >= 5 for non-null, >= 3 for null); no state change.
// CONFIDENCE: MEDIUM
if (Portal != nullptr)
{
	TargetPortal = Portal;
	// Verbosity-gated diagnostic log at 0x142c54b70 elided (no state change)
}
else
{
	// Verbosity-gated diagnostic log at 0x142c54b70 elided (no state change)
}
