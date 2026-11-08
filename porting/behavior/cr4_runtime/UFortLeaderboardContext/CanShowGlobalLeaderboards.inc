// Behaviour snippet: UFortLeaderboardContext::CanShowGlobalLeaderboards (body only; spliced into the generated member function).
// Original address: 0x144cfb850 (thunk 0x1452801e0)
// Evidence: UFortGlobals = NativeGetGlobals() (0x141ed2350); the whole byte at +0x204 is loaded (MOVZX) and returned as bool. +0x204 is the declared bitfield byte of bGlobalLeaderboardsFrontEndEnabled (mask 1) and the asm reads the full byte, so it is read raw at the object offset.
// CONFIDENCE: HIGH
static_assert(sizeof(UFortGlobals) > 0, "Generated header include for UFortGlobals.");
const uint8* GlobalsRaw = reinterpret_cast<const uint8*>(UFortGlobals::NativeGetGlobals());
return GlobalsRaw[0x204] != 0;
