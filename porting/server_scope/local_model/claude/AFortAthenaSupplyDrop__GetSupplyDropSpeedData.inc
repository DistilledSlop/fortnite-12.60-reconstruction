// CONFIDENCE: MEDIUM. Original address: 0x142833f90 (exec thunk). Helper: 0x141918b00 (captured, read in full).
// Behaviour: when the GameMode is a FortGameModeAthena, first ask the registered speed providers (vslot 0x780) in order,
// then the GameMode phase config (0x141949b10). Fallback 280 / 500.
MinSpeed = 280.0f;
MaxSpeed = 500.0f;

if (UWorld* World = GetWorld())
{
    AGameModeBase* GameMode = World->AuthorityGameMode;
    if (GameMode != nullptr && GameMode->IsA(AFortGameModeAthena::StaticClass()))
    {
        GameMode->GetSupplyDropSpeedRange(this, MinSpeed, MaxSpeed);   // FUN_141949b10
    }
}
