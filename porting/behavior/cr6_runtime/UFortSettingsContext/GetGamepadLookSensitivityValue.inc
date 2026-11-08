// Original address: 0x144d03e80 (thunk 0x1452ce300)
// Offset evidence: this+0x34 is below UFortSettingsContext's lowest own offset (0x3c), so it is raw inherited storage (float). Record +0x2d4 = GamepadLookSensitivity (FVector2D, X at +0x2d4, Y at +0x2d8). Constant [0x1459a86f8] is 1.0f in .rdata (checked in the PE image). Null record returns (0,0) (MOV qword [RBX],RAX with RAX=0). Native body writes through the hidden RDX out pointer and returns it in RAX; the port returns the FVector2D value.
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be a complete type");
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (!SettingsRecord)
{
    return FVector2D(0.0f, 0.0f);
}
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const float Divisor = *reinterpret_cast<const float*>(Raw + 0x34);
const float LookX = SettingsRecord->GamepadLookSensitivity.X;
const float LookY = SettingsRecord->GamepadLookSensitivity.Y;
// JNC on COMISS a,1.0: values at or above 1.0 (ordered) take MINSS(Divisor, a) = (Divisor < a) ? Divisor : a; otherwise the clamp is 1.0.
const float ClampX = (LookX >= 1.0f) ? ((Divisor < LookX) ? Divisor : LookX) : 1.0f;
const float ClampY = (LookY >= 1.0f) ? ((Divisor < LookY) ? Divisor : LookY) : 1.0f;
const float Inverse = 1.0f / Divisor;
return FVector2D(Inverse * ClampX, Inverse * ClampY);
