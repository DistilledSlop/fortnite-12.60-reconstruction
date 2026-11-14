// CONFIDENCE: HIGH; complete original cull-distance math helper.
// Original 0x141fbc430. WorldSettings may be null; the fallback is the live FortGlobals singleton.
const UFortGlobals* Globals = WorldSettings ? nullptr : UFortGlobals::NativeGetGlobals();
const float MinSize = WorldSettings ? WorldSettings->MinCullObjectSize : Globals->MinCullObjectSize;
const float MaxSize = WorldSettings ? WorldSettings->MaxCullObjectSize : Globals->MaxCullObjectSize;
const float MinDistance = WorldSettings ? WorldSettings->MinCullDistance : Globals->MinCullDistance;
const float MaxDistance = WorldSettings ? WorldSettings->MaxCullDistance : Globals->MaxCullDistance;
float ClampedSize;
if (MinSize <= ObjectSize)
{
    ClampedSize = ObjectSize <= MaxSize ? ObjectSize : MaxSize;
}
else
{
    ClampedSize = MinSize;
}
return logf(((ClampedSize - MinSize) / (MaxSize - MinSize)) * 9.0f + 1.0f)
    * 0.43429446f * (MaxDistance - MinDistance) + MinDistance;
