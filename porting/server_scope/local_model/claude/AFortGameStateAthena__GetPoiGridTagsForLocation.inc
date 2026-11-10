// CONFIDENCE: HIGH. Exec thunk 0x14291f300 forwards Location and the out container to virtual slot 0x8f8, whose native body is 0x141e9e640: it zeroes the output container and, when the POI manager (this+0x308) exists, asks it for the grid tags at Location (AFortPoiManager::GetPoiGridTagsForLocation, 0x141a470e0, deps AFortPoiManager__GetPoiGridTagsForLocation).
// Original address: 0x14291f300 (exec). Native body: 0x141e9e640 (vtable slot 0x8f8 on AFortGameStateAthena).
FGameplayTagContainer OutGameplayTags;
if (PoiManager != nullptr)
{
	PoiManager->GetPoiGridTagsForLocation(Location, OutGameplayTags);
}
return OutGameplayTags;
