// Behaviour snippet: UFortHUDContext::GetScoreDisplayFactor (body only; spliced into the generated member function).
// Original address: 0x144cd0aa0 (thunk 0x145256d20)
// Evidence: UGameDataSTW = NativeGetGameDataSTW() (0x1424e1120); returns the float at +0xc8 (ScoreDisplayFactor).
// CONFIDENCE: HIGH
static_assert(sizeof(UGameDataSTW) > 0, "Generated header include for UGameDataSTW.");
return UGameDataSTW::NativeGetGameDataSTW()->ScoreDisplayFactor;
