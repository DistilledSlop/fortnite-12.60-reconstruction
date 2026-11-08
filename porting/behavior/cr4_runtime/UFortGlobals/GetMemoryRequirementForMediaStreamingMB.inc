// Behaviour snippet: UFortGlobals::GetMemoryRequirementForMediaStreamingMB (body only; spliced into the generated member function).
// Original address: 0x141ed6b20 (thunk 0x142934030)
// Evidence: UFortGlobals = NativeGetGlobals() (0x141ed2350); returns the Int64 at +0x2a8 (MemoryRequirementForMediaStreamingMB).
// Verbosity-gated diagnostic log at 0x142c54b70 (guarded by CMP byte [0x1481348e0],5) elided (no state change).
// CONFIDENCE: MEDIUM
static_assert(sizeof(UFortGlobals) > 0, "Generated header include for UFortGlobals.");
return UFortGlobals::NativeGetGlobals()->MemoryRequirementForMediaStreamingMB;
