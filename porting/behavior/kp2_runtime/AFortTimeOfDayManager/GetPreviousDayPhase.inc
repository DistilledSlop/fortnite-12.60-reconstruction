// Behaviour snippet: AFortTimeOfDayManager::GetPreviousDayPhase (body only; spliced into the generated member function).
// Original body address: 0x141f60120 (thunk 0x142b0ef50)
// Offset evidence: this+0x228 = CurrentDayNightPhase (Byte, reflected). Asm jump table: 0->3, 1->0, 2->1, 3->2, 4 -> 4, other values -> 4.
// Verbosity-gated diagnostic logs (call 0x142c54b70 guarded by a CMP against a global byte) elided (no state change).
// CONFIDENCE: MEDIUM
const uint8 CurrentPhase = static_cast<uint8>(CurrentDayNightPhase);
uint8 PreviousPhase = 4;
switch (CurrentPhase)
{
    case 0: PreviousPhase = 3; break;
    case 1: PreviousPhase = 0; break;
    case 2: PreviousPhase = 1; break;
    case 3: PreviousPhase = 2; break;
    default: break;
}
return static_cast<EFortDayPhase::Type>(PreviousPhase);
