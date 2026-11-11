// CONFIDENCE: LOW Original addresses: thunk 0x142b46500; see evidence.json for callees.
// Signature: bool GetPickupInstigatorData(int32 InstigatorHandle, FFortPickupInstigatorData& InstigatorData)
// Native 0x14269e130. Authority only; resolves this weapon's ItemEntryGuid through the owner's instigator registry, then fetches the matching entry's data.
if (Role != ROLE_Authority)
{
    return false;
}
return NativeFindPickupInstigatorData(ItemEntryGuid, InstigatorHandle, InstigatorData);
