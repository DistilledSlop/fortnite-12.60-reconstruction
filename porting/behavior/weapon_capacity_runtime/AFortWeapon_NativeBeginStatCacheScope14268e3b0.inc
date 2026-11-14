// CONFIDENCE: HIGH. ScopeWeapon is the actual one-pointer scope storage (reference return preserves this address).
// A disabled/null input leaves that storage unchanged, matching the native constructor helper.
if (Weapon && Weapon->bNativeAllowStatCaching201)
{
    ScopeWeapon = Weapon;
    Weapon->NativeStatCacheScopeDepth201 = static_cast<int32>(
        static_cast<uint32>(Weapon->NativeStatCacheScopeDepth201) + 1u);
}
return ScopeWeapon;
