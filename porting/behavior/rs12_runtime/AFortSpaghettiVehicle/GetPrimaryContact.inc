// Original body 0x1419d60b0 (thunk 0x142a08980). Offset evidence: bool byte at +0x1290 (member NativeRecoveredBool_0x1290); FVector at +0x1244 (ContactPoint source) and FVector at +0x1250 (ContactNormal source), both unreflected gaps in the class's own region.
// Vectors are copied only when bIstouching is set.
// CONFIDENCE: HIGH
bIstouching = NativeRecoveredBool_0x1290;
if (bIstouching)
{
    ContactPoint = NativeRecoveredFVector_0x1244;
    ContactNormal = NativeRecoveredFVector_0x1250;
}
