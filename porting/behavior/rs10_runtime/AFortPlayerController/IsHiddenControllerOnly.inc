// Behaviour snippet: AFortPlayerController::IsHiddenControllerOnly (body only; spliced into the generated member function).
// Original body address: 0x14238e8a0 (thunk 0x142a480b0, group B, flags 0x54020401).
// Offset evidence: this+0x2e8 = HiddenActors (APlayerController Array, reflected, data pointer); this+0x2f0 = its Num (dword). The engine header is not generated, so the array is read raw. Elements are 8-byte pointers compared by identity; a found index is never -1, so the found path returns true.
// CONFIDENCE: HIGH
if (TargetActor == nullptr)
{
    return false;
}
const uint8* Raw = reinterpret_cast<const uint8*>(this);
AActor* const* Elements = *reinterpret_cast<AActor* const* const*>(Raw + 0x2e8);
const int32 ElementCount = *reinterpret_cast<const int32*>(Raw + 0x2f0);
for (int32 Index = 0; Index < ElementCount; ++Index)
{
    if (Elements[Index] == TargetActor)
    {
        return true;
    }
}
return false;
