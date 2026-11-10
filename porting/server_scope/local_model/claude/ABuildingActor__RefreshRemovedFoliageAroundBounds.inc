// CONFIDENCE: MEDIUM. Refreshes foliage removed by this building: first the building-side foliage removal (virtual 0x988), then the bounds-based refresh (virtual 0x980) that unions the bounds of excluded primitives and forwards them to the foliage subsystem.
// Original address: 0x142737e30 (exec). Native body: 0x141bc24b0 (virtual slots 0x988 then 0x980).
RemoveFoliageForBuilding();
RefreshRemovedFoliageInBounds(bUseNonCollidingBounds);
