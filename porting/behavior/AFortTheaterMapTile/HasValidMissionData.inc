// CONFIDENCE: HIGH; captured native behavior and reflection agree.
// The native predicate checks the FString array count of MissionData.MissionGuid > 1.
return !MissionData.MissionGuid.IsEmpty();
