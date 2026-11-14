// CONFIDENCE: HIGH. Complete original body; class lifecycle acceptance remains separate.
// Native 0x141b1e0e0 reads byte +0x35a, an unreflected SpecialEvent member.
// Constructor 0x141b12500 initializes it; producer 0x141b18370 owns later updates.
if (NativeADSOverrideByte035a)
{
    bAllow = true;
    return 2;
}
return 0;
