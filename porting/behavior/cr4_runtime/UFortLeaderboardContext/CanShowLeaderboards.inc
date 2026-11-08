// Behaviour snippet: UFortLeaderboardContext::CanShowLeaderboards (body only; spliced into the generated member function).
// Original address: 0x144cfb870 (thunk 0x145280210)
// Evidence: UFortGlobals = NativeGetGlobals() (0x141ed2350); the whole byte at +0x1fe is loaded (MOVZX) and returned as bool. +0x1fe is the declared bitfield byte of bAthenaLeaderboardFrontEndEnabled (mask 1) and the asm reads the full byte, so it is read raw at the object offset.
// CONFIDENCE: HIGH
static_assert(sizeof(UFortGlobals) > 0, "Generated header include for UFortGlobals.");
const uint8* GlobalsRaw = reinterpret_cast<const uint8*>(UFortGlobals::NativeGetGlobals());
return GlobalsRaw[0x1fe] != 0;
