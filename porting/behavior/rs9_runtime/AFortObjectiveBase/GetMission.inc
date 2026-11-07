// Original body: 0x14213aa20 (thunk 0x142a049f0)
// Offset evidence: AActor::Owner Object at +0xe0 (reflected); IsA check against AFortMission::StaticClass (candidate 0x1429da8e0); returns Owner when the class test matches, else null.
// CONFIDENCE: HIGH
if (GetOwner() == nullptr)
{
    return nullptr;
}
if (!GetOwner()->IsA(AFortMission::StaticClass()))
{
    return nullptr;
}
return static_cast<AFortMission*>(GetOwner());
