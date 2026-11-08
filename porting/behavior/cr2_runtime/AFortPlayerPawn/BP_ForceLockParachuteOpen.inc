// Original body: 0x1423061d0 (thunk 0x142a7ae10). Offset evidence: [RCX+0xf0] is AActor::Role (private in UE 4.25, read via GetLocalRole(); ROLE_Authority = 3); [RCX+0x2284] is AFortPlayerPawn::bParachuteLockedOpen and [RCX+0x2285] is bLocalParachuteLockedOpen (reflected bool members, offsets 0x2284/0x2285 per bool_bits.json); both are byte stores of the bLocked argument (DL).
// CONFIDENCE: HIGH
if (GetLocalRole() == ROLE_Authority)
{
	bParachuteLockedOpen = bLocked;
	bLocalParachuteLockedOpen = bLocked;
}
