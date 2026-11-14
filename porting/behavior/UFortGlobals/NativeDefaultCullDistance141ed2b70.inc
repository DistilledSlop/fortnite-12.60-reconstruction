// CONFIDENCE: HIGH; complete original cull-distance math helper.
// Original 0x141ed2b70: the live global bounds are read on every call.
const UFortGlobals* Globals = UFortGlobals::NativeGetGlobals();
return (Globals->MaxCullDistance + Globals->MinCullDistance) * 0.5f;
