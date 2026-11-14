// CONFIDENCE: HIGH. Native scope destructor; clear all query caches at the outermost scope exit.
if (ScopeWeapon)
{
    ScopeWeapon->NativeStatCacheScopeDepth201 = static_cast<int32>(
        static_cast<uint32>(ScopeWeapon->NativeStatCacheScopeDepth201) - 1u);
    if (ScopeWeapon->NativeStatCacheScopeDepth201 == 0)
        ScopeWeapon->NativeInvalidateStatCaches1426afa80();
    ScopeWeapon = nullptr;
}
