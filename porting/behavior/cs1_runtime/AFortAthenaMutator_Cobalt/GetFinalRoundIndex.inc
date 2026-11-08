// Original body address: 0x141aad030 (thunk 0x1427f5dc0)
// Offset evidence: this+0x3f0 is an unreflected signed byte (MOVSX) in the class's own region; emitted as NativeRecoveredUint8_0x3f0.
// Constants: the scale 1.442695f (1/ln 2) is the function-local static initialised on first call (decompile packet porting/generated/work-packets/0x141aad030.md); the offset is 1.0f.
// The MSVC guard and TLS check are the magic-static initialisation idiom and are not ported. Result is (int32)(logf(float(byte)) * scale - 1.0f) truncated.
// CONFIDENCE: MEDIUM
static float LogScale = 1.442695f;
const int8 RoundByte = static_cast<int8>(NativeRecoveredUint8_0x3f0);
const float LogValue = logf(static_cast<float>(RoundByte));
return static_cast<int32>(LogValue * LogScale - 1.0f);
