// Original body address: 0x141fe9400
// Offset evidence: ItemID (FName) at +0x34 (ComparisonIndex) and +0x38 (Number). Asm returns this only when _ItemID matches ItemID, and returns null for FName None (0,0).
// CONFIDENCE: HIGH
if (_ItemID == FName()) { return nullptr; }
if (ItemID == _ItemID) { return this; }
return nullptr;
