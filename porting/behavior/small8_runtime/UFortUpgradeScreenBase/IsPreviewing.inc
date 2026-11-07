// Original body address: 0x145129e90
// Offset evidence: Details is the reflected object at +0x428. Its pointee (class unresolved) has an object pointer at +0x260, whose pointee has a byte at +0x66 (same byte as UFortUpgradeInfo 0x66). Null guards on Details are added; the original has none.
// CONFIDENCE: MEDIUM
if (Details == nullptr)
{
    return false;
}
const uint8* Info = *reinterpret_cast<const uint8* const*>(reinterpret_cast<const uint8*>(Details) + 0x260);
if (Info == nullptr)
{
    return false;
}
return Info[0x66] != 0;
