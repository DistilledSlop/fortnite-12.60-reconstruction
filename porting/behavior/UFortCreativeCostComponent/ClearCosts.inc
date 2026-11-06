// CONFIDENCE: high
// Native body at 0x1428b9030 checks owner role (via 0x1440b2120); if authority, initializes ItemCosts to 6 zeros.
if (const AActor* Owner = GetOwner())
{
	if (Owner->GetLocalRole() == ROLE_Authority)
	{
		ItemCosts.Init(0, 6);
	}
}
