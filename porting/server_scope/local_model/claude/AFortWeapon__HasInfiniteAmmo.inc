// CONFIDENCE: MEDIUM Original addresses: thunk 0x142b472e0; see evidence.json for callees.
// Signature: bool HasInfiniteAmmo() const
// Callee 0x1426a24a0. Infinite when the weapon does not use an ammo counter, or the inventory owner reports infinite ammo for this weapon data.
IFortInfiniteAmmoProvider* Provider = NativeGetInfiniteAmmoProvider();
const bool bOwnerInfinite = Provider ? Provider->IsInfiniteAmmoForWeaponData(WeaponData) : false;
return !NativeUsesAmmoCounter() || bOwnerInfinite;
