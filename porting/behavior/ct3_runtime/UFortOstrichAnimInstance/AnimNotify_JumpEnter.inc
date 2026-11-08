// Original body 0x141833820 (thunk 0x142a0b040).
// Offset evidence: [this+0x5d0] = ChargeJumpFloatValue (reflected float), compared against 0.6f with JA.
// Four qword slots at 0x668..0x687 (Charged jump sequences) or 0x688..0x6a7 (Normal jump sequences) are copied to the JumpBase slots 0x6a8..0x6c7.
// Slots are copied as raw qwords because their pointee types differ (UAnimSequence and UBlendSpace1D).
// CONFIDENCE: HIGH
const int32 SourceOffset = (0.6f > ChargeJumpFloatValue) ? 0x688 : 0x668;
uint8* Raw = reinterpret_cast<uint8*>(this);
for (int32 Slot = 0; Slot < 4; ++Slot)
{
	*reinterpret_cast<uint64*>(Raw + 0x6a8 + Slot * 8) = *reinterpret_cast<const uint64*>(Raw + SourceOffset + Slot * 8);
}
