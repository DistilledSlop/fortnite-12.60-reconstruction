// Behaviour snippet: AFortVehicleManager::GetVehicleCountByTag (body only; spliced into the generated member function).
// Original body address: 0x141f8ab40 (thunk 0x142b2e8c0)
// Offset evidence: this+0x250 = VehicleCountByTagMap (TMap<FGameplayTag,int32>, reflected). Body is a TSet hash probe keyed by the tag FName index (hash helper 0x142ce11c0); a miss returns 0.
// Access pattern matches TMap::FindRef (value at element+8 when found, else 0). Tag equality in the asm compares only the FName ComparisonIndex, so this differs only for tags sharing an index with a different Number.
// CONFIDENCE: MEDIUM
return VehicleCountByTagMap.FindRef(VehicleTag);
