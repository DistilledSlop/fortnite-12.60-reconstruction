// CONFIDENCE: high
// 0x141d09870. Preserve both lower and upper bounds checks.
return PlayerSlots.IsValidIndex(Seat) ? PlayerSlots[Seat].Player : nullptr;
