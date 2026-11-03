// CONFIDENCE: HIGH
bool bRuleInfiniteAmmo = false;
UWorld* World = GetWorld();
if (World)
{
    AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(World->GetGameState());
    if (GameState)
    {
        AFortPlayerStateAthena* AthenaPlayerState = Cast<AFortPlayerStateAthena>(PlayerState);
        bRuleInfiniteAmmo = AFortGameStateAthena::NativeQueryInfiniteAmmoRule(GameState, AthenaPlayerState, WeaponDefinition);
    }
}
return bInfiniteAmmo || bRuleInfiniteAmmo;
