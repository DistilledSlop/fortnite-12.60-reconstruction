// CONFIDENCE: high; native 0x141a5f1c0, preserving override and first-match order.
EDeathCause Cause = EDeathCause::Unspecified;
UFortGameData* GameData = UFortAssetManager::NativeGetFortAssetManager().NativeGetCommonGameData();
auto& MappingRef = GameData->FortDeathCauseFromTagMapping;
if (!MappingRef.IsNull())
{
    UFortDeathCauseFromTagMapping* Mapping = MappingRef.Get();
    if (!Mapping)
        Mapping = Cast<UFortDeathCauseFromTagMapping>(FortLoadDelayedObject(MappingRef.ToSoftObjectPath(), true));
    FFortAssets::Get().TrackAsset(Mapping);
    if (Mapping)
    {
        for (const FFortTagToDeathCause& Row : Mapping->TagToDeathCauseArray)
        {
            if (InTags.HasTag(Row.DeathTag))
            {
                Cause = bWasDBNO ? Row.DBNOCause : Row.DeathCause;
                break;
            }
        }
    }
}
if (Cause != EDeathCause::Unspecified)
    return Cause;
const auto HasNativeTag = [&InTags](uint32 Offset)
{
    return InTags.HasTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(Offset));
};
if (HasNativeTag(0x1628))
    return EDeathCause::ShoppingCart;
if (HasNativeTag(0x1638))
    return EDeathCause::ATK;
if (HasNativeTag(0x1640))
    return EDeathCause::QuadCrasher;
if (HasNativeTag(0x1648))
    return EDeathCause::Biplane;
if (HasNativeTag(0xb00))
    return EDeathCause::BiplaneGun;
if (HasNativeTag(0x3b0))
    return EDeathCause::Vehicle;
if (HasNativeTag(0x3b8))
    return EDeathCause::Cube;
if (HasNativeTag(0x3c0))
    return EDeathCause::StormSurge;
if (HasNativeTag(0x420))
    return EDeathCause::InstantEnvironmentalFellOutOfWorld;
if (HasNativeTag(0x428))
    return EDeathCause::InstantEnvironmentalUnderLandscape;
if (HasNativeTag(0x418))
    return EDeathCause::InstantEnvironmental;
if (HasNativeTag(0x358))
    return bWasDBNO ? EDeathCause::DBNOTimeout : EDeathCause::FallDamage;
if (HasNativeTag(0x360))
    return bWasDBNO ? EDeathCause::DBNOTimeout : EDeathCause::OutsideSafeZone;
if (HasNativeTag(0xb20))
    return EDeathCause::MassiveMelee;
if (HasNativeTag(0xb30))
    return EDeathCause::MassiveDiveBomb;
if (HasNativeTag(0xb38))
    return EDeathCause::MassiveRanged;
if (HasNativeTag(0xb28))
    return EDeathCause::InfinityBlade;
if (HasNativeTag(0x9e0))
    return EDeathCause::Melee;
if (HasNativeTag(0xa00))
    return EDeathCause::Pistol;
if (HasNativeTag(0xaa0))
    return EDeathCause::SMG;
if (HasNativeTag(0xa30))
    return EDeathCause::Shotgun;
if (HasNativeTag(0xa50))
    return EDeathCause::Rifle;
if (HasNativeTag(0xa80))
    return HasNativeTag(0x638) ? EDeathCause::Sniper : EDeathCause::SniperNoScope;
if (HasNativeTag(0xad8))
    return EDeathCause::Grenade;
if (HasNativeTag(0xae0))
    return EDeathCause::C4;
if (HasNativeTag(0xae8))
    return EDeathCause::GrenadeLauncher;
if (HasNativeTag(0xaf0))
    return EDeathCause::RocketLauncher;
if (HasNativeTag(0xaa8))
    return EDeathCause::Bow;
if (HasNativeTag(0xab8))
    return EDeathCause::Minigun;
if (HasNativeTag(0xac0))
    return EDeathCause::LMG;
if (HasNativeTag(0xac8))
    return EDeathCause::Turret;
if (HasNativeTag(0xad0))
    return EDeathCause::ShipCannon;
if (HasNativeTag(0x5a8))
    return EDeathCause::Trap;
if (HasNativeTag(0xb40))
    return EDeathCause::GasGrenade;
if (HasNativeTag(0x380))
    return EDeathCause::TeamSwitchSuicide;
if (HasNativeTag(0x450))
    return EDeathCause::WonMatch;
if (HasNativeTag(0xef0))
    return EDeathCause::BasicFiend;
if (HasNativeTag(0xef8))
    return EDeathCause::EliteFiend;
if (HasNativeTag(0xf00))
    return EDeathCause::RangedFiend;
if (HasNativeTag(0xf10))
    return EDeathCause::BasicBrute;
if (HasNativeTag(0xf18))
    return EDeathCause::EliteBrute;
if (HasNativeTag(0xf20))
    return EDeathCause::MegaBrute;
if (HasNativeTag(0x2d0))
    return EDeathCause::Lava;
if (HasNativeTag(0x368))
    return EDeathCause::DBNOTimeout;
if (HasNativeTag(0x448))
    return EDeathCause::SilentSwitchingToSpectate;
return EDeathCause::Unspecified;
