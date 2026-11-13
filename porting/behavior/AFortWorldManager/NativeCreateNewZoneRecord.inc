// CONFIDENCE: HIGH
// Original 0x141f7c620 is the generic NewObject<UFortZoneRecord> helper.
// Its caller supplies the world manager as Outer; preserve the explicit argument.
return NewObject<UFortZoneRecord>(Outer);
