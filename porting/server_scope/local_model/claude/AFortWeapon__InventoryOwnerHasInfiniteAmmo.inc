// CONFIDENCE: MEDIUM Original addresses: thunk 0x142b47310; see evidence.json for callees.
// Signature: bool InventoryOwnerHasInfiniteAmmo() const
// Callee 0x1426a58f0. Returns the inventory owner's infinite-ammo answer for this weapon data.
IFortInfiniteAmmoProvider* Provider = NativeGetInfiniteAmmoProvider();
return Provider ? Provider->IsInfiniteAmmoForWeaponData(WeaponData) : false;
