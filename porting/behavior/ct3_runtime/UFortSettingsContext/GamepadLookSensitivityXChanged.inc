// Original body 0x144d00710 (thunk 0x1452cd6a0).
// Offset evidence: [this+0x34] = base sensitivity float (below the class lowest own offset 0x3c, raw access).
// Record = NativeGetSettingsRecord144d03d50(); GamepadLookSensitivity (FVector2D at 0x2d4, X at 0x2d4) is reflected.
// bDirty (bool bitfield at 0x87e, mask 1) is set when the value changes by more than 1e-8.
// Clamp: scaled >= 1.0 selects min(base, scaled); otherwise 1.0 (JC path, NaN included).
// CONFIDENCE: HIGH
UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
if (Record == nullptr)
{
	return;
}
const float BaseSensitivity = *reinterpret_cast<const float*>(reinterpret_cast<const uint8*>(this) + 0x34);
const float ScaledValue = BaseSensitivity * NewValue;
float ClampedValue = 1.0f;
if (ScaledValue >= 1.0f)
{
	ClampedValue = (BaseSensitivity < ScaledValue) ? BaseSensitivity : ScaledValue;
}
if (FMath::Abs(ClampedValue - Record->GamepadLookSensitivity.X) > 1e-8f)
{
	Record->GamepadLookSensitivity.X = ClampedValue;
	Record->bDirty = true;
}
