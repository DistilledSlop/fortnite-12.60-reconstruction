// Behaviour snippet: UFortGlobals::SetGlobalSnowAltitude (body only; spliced into the generated member function).
// Original address: 0x141edc400 (thunk 0x142934c90)
// Evidence: NativeGetGlobals() (0x141ed2350), then the float Altitude (XMM0) is stored at +0x2cc (GlobalSnowAltitude).
// CONFIDENCE: HIGH
static_assert(sizeof(UFortGlobals) > 0, "Generated header include for UFortGlobals.");
UFortGlobals::NativeGetGlobals()->GlobalSnowAltitude = Altitude;
