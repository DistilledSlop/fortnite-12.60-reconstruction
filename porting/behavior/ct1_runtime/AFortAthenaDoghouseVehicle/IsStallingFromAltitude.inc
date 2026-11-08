// Body 0x1418f1bd0 (thunk 0x1427d80f0). Offset evidence: [this+0x130] is the inherited AActor root component pointer (engine storage, raw access); root+0x1d8 is the world Z of the root transform (raw, pointee not this class); [this+0x1238] is an unreflected own-region pointer (NativeRecoveredPtr_0x1238) whose float at +0x4a8 is the threshold.
// A null root takes the zero-valued writable fallback (0x148218638 == 0.0f), so Z is 0.0f on that path.
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const uint8* Root = *reinterpret_cast<const uint8* const*>(Raw + 0x130);
float LocalZ = 0.0f;
if (Root != nullptr)
{
    LocalZ = *reinterpret_cast<const float*>(Root + 0x1d8);
}
const uint8* Mv = reinterpret_cast<const uint8*>(NativeRecoveredPtr_0x1238);
return LocalZ > *reinterpret_cast<const float*>(Mv + 0x4a8);
