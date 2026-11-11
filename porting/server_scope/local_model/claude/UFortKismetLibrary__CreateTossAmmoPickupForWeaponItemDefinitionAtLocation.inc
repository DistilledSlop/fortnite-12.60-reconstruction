// CONFIDENCE: MEDIUM. Spawns a toss-ammo pickup for WeaponItemDefinition at Location in the world of WorldContextObject, using the toss-ammo pickup class from the asset manager. Nothing spawns when the world, the weapon definition or the class is missing. SourceTags, SourceTypeFlag and SpawnSource pass through to the spawn.
// Original address: 0x1429686a0 (exec). Native body: 0x141eea600. Helpers: 0x144640410 (world), 0x141a777c0 (asset manager), 0x141a79670 (toss-ammo pickup class), 0x141f87780 (spawn), 0x141f85500 (spawn core), 0x14203cda0 (set location).
if (WorldContextObject == nullptr || WeaponItemDefinition == nullptr)
{
    return;
}
UWorld* World = GetWorldFromContextObject(WorldContextObject);
if (World == nullptr)
{
    return;
}
UFortAssetManager* AssetManager = UFortAssetManager::Get();
TSubclassOf<AFortPickup> TossAmmoPickupClass = AssetManager->GetTossAmmoPickupClass();
if (TossAmmoPickupClass == nullptr)
{
    return;
}
SpawnTossAmmoPickup(TossAmmoPickupClass, World, WeaponItemDefinition, SourceTags, Location, SourceTypeFlag, SpawnSource);
