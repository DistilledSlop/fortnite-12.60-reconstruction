// CONFIDENCE: HIGH
// Original address: 0x142a7bca0 (exec thunk, no parameters, returns float). Calls FUN_142319870(this).
// Captures: captures/13799d83931e48b19e2b5ccf2a1528b5/0x142a7bca0.c ; FUN_142319870 captures/a07ef25a9e7146a9946d3e2dec97791c/0x142319870.c ;
// base aim rotation getter slot 0x6d8 -> 0x1423198d0 (read in full, captures porting/generated/native-cap-virtual3/0x1423198d0.c).
// FUN_142bc0d80 is a float remainder (fmod-like): its xmm1 divisor is not visible in the decompile. 360 is inferred from the quantization constants.
FRotator BaseAimRotation = GetBaseAimRotation();   // slot 0x6d8; Pitch in the low float
float Pitch = FMath::Fmod(BaseAimRotation.Pitch, 360.0f);   // FUN_142bc0d80
if (Pitch < 0.0f)
{
	Pitch += 360.0f;
}
// Quantize to 256 steps over [0, 360): 0.7083334 = 255/360 (approx), 1.4117647 = 360/255.
const int32 QuantizedStep = static_cast<int32>(Pitch * 0.7083334f) & 0xff;
return static_cast<float>(QuantizedStep) * 1.4117647f;
