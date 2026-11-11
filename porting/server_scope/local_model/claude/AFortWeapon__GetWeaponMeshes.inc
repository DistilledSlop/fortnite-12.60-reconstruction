// CONFIDENCE: MEDIUM Original addresses: thunk 0x142b471b0; see evidence.json for callees.
// Signature: void GetWeaponMeshes(TArray<UObject*>& WeaponMeshes)
// Native 0x1426a1500. Mesh list is collected once per weapon and cached in AllWeaponMeshes.
if (!bHasCachedAdditionalMeshes)
{
    AllWeaponMeshes.Reset();
    // Helper proposed: NativeCollectAdditionalWeaponMeshes (native 0x140860090 when a world is found, otherwise no-op 0x14085fd60).
    NativeCollectAdditionalWeaponMeshes(AllWeaponMeshes);
    bHasCachedAdditionalMeshes = true;
}
WeaponMeshes = AllWeaponMeshes;
