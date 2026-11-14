// CONFIDENCE: HIGH
// Original 0x142788290. Original native struct ops call the base hit serializer, then CartridgeID and WeaponInfo.
FGameplayAbilityTargetData_SingleTargetHit::NetSerialize(Ar, Map, bOutSuccess);
Ar << CartridgeID;
Ar << WeaponInfo;
return true;
