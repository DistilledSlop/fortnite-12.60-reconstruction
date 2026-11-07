// Original body: 0x1419d60b0 (thunk 0x142a08980)
// Offset evidence: bool at +0x1290 (NativeRecoveredBool_0x1290); ContactPoint floats at +0x1244, +0x1248, +0x124c; ContactNormal floats at +0x1250, +0x1254, +0x1258. No reflected member covers these offsets in the class own region (min 0xf38).
// CONFIDENCE: HIGH
bIstouching = NativeRecoveredBool_0x1290;
if (!NativeRecoveredBool_0x1290)
{
    return;
}
ContactPoint.X = NativeRecoveredFloat_0x1244;
ContactPoint.Y = NativeRecoveredFloat_0x1248;
ContactPoint.Z = NativeRecoveredFloat_0x124c;
ContactNormal.X = NativeRecoveredFloat_0x1250;
ContactNormal.Y = NativeRecoveredFloat_0x1254;
ContactNormal.Z = NativeRecoveredFloat_0x1258;
