// Original body 0x14269bad0 (thunk 0x142b45d10).
// Offset evidence: sret (RDX) is zeroed from writable snapshot 0x148218630/0x148218638 (0.0, ZeroVector) then
// NativeDecodeDamageStartVectors142119f50(RCX=this, RDX=sret as OutFirst, R8=AimDir as OutSecond) is called.
// R8 is the first declared parameter (AimDir) per header_decl; PatternIndex (R9) is never read by the body.
// CONFIDENCE: MEDIUM
FVector Result = FVector::ZeroVector;
AFortWeapon::NativeDecodeDamageStartVectors142119f50(const_cast<AFortWeapon*>(this), Result, AimDir);
(void)PatternIndex;
return Result;
