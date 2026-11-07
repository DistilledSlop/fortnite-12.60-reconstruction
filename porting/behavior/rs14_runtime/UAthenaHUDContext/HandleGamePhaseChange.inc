// Behaviour snippet: UAthenaHUDContext::HandleGamePhaseChange (body only; spliced into the generated member function).
// Original address: 0x144c92ae0 (thunk 0x14519e3c0)
// Offset evidence: this+0xe8 is reflected as OnAthenaGamePhaseChanged (InlineMulticastDelegate, UAthenaHUDContext.h). The body copies the TScriptInterface pair from RDX ([RDX], [RDX+8]) and GamePhaseStep from R8B into a packed 17-byte param struct, then calls 0x1406a9dc0 (delegate broadcast) with RCX = this+0xe8 and RDX = that struct. Delegate FOnAthenaGamePhaseChanged (FOnAthenaGamePhaseChanged.h) takes (const TScriptInterface<IFortSafeZoneInterface>&, EAthenaGamePhaseStep).
// CONFIDENCE: HIGH
OnAthenaGamePhaseChanged.Broadcast(SafeZoneInterface, GamePhaseStep);
