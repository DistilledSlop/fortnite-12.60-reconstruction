// Original body 0x144d06700 (thunk 0x1452ce7e0).
// Offset evidence: [this+0x30] = float divisor (below the class lowest own offset 0x3c, raw access).
// Record = NativeGetSettingsRecord144d03d50(); MouseSensitivityX (0x30c) and MouseSensitivityY (0x310) are reflected floats.
// Result starts at the writable-snapshot zeros (0x148218628/0x14821862c), so zero is the fallback.
// Each axis is clamped to [0, 1]: a negative or NaN scaled value gives 0.
// CONFIDENCE: MEDIUM
FVector2D Result(0.0f, 0.0f);
UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
if (Record == nullptr)
{
	return Result;
}
const float InvDivisor = 1.0f / *reinterpret_cast<const float*>(reinterpret_cast<const uint8*>(this) + 0x30);
const float ScaledX = InvDivisor * Record->MouseSensitivityX;
float ClampedX = 0.0f;
if (ScaledX >= 0.0f)
{
	ClampedX = (ScaledX < 1.0f) ? ScaledX : 1.0f;
}
Result.X = ClampedX;
const float ScaledY = InvDivisor * Record->MouseSensitivityY;
float ClampedY = 0.0f;
if (ScaledY >= 0.0f)
{
	ClampedY = (ScaledY < 1.0f) ? ScaledY : 1.0f;
}
Result.Y = ClampedY;
return Result;
