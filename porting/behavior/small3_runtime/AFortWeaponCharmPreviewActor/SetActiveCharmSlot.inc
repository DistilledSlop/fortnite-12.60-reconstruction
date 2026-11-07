// Original body: 0x141dd79d0 (thunk 0x142b4c2d0)
// Offset evidence: CMP EDX,3; JA skips; MOV byte [RCX+0x2c8],DL
// CONFIDENCE: HIGH
if (static_cast<uint32>(CharmSlotIn) <= 3u)
{
    CharmSlot = static_cast<decltype(CharmSlot)>(CharmSlotIn);
}
