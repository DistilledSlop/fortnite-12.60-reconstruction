// CONFIDENCE: HIGH
// Original 0x14169e1e0.
if (NativeReadPhoebeInfiniteAmmo141a49680() && CacheInventoryDigestedSkillSet)
{
    if (NativeCachedGamePhase == EAthenaGamePhase::Warmup || !WeaponDefinition)
    {
        return false;
    }
    UFortWeaponItemDefinition* Definition = Cast<UFortWeaponItemDefinition>(WeaponDefinition);
    if (!Definition || Definition->NativeGetItemType() != EFortItemType::WeaponRanged)
    {
        return false;
    }
    UFortAthenaAIBotInventoryDigestedSkillSet* Skills = CacheInventoryDigestedSkillSet;
    if (!Skills->NativeGetInfiniteAmmoCheatEnabled1418a1010(Definition))
    {
        return false;
    }
    Skills = CacheInventoryDigestedSkillSet;
    if (Skills->NativeGetCheckLoadedAmmoForInfiniteAmmoCheat1418a0720(Definition))
    {
        UFortItem* Item = NativeFindItemA0(Definition, false);
        if (Item && Item->GetLoadedAmmo() > 0)
        {
            return false;
        }
    }
    UFortWorldItemDefinition* Ammo = Definition->GetAmmoWorldItemDefinitionNative();
    if (!Ammo)
    {
        return true;
    }
    UFortItem* Item = NativeFindItemA0(Ammo, false);
    return !Item || Item->GetNumInStack() <= int32(Ammo == Definition);
}
UWorld* World = GetWorld();
if (!World)
{
    return false;
}
AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(World->GetGameState());
if (!GameState)
{
    return false;
}
AFortPlayerStateAthena* AthenaPlayerState = Cast<AFortPlayerStateAthena>(PlayerState);
return AFortGameStateAthena::NativeQueryInfiniteAmmoRule(GameState, AthenaPlayerState, WeaponDefinition);
