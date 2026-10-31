// Behaviour snippet: AFortGameStateAthena::OnRep_MutatorEventData (body only; spliced into the generated member function).
// Original address: 0x142920550
// Decompile source: porting/generated/decomp_improved/AFortGameStateAthena/OnRep_MutatorEventData.c
// Proxy score: 56.8 (portability.json)
// Inferred: Decompile calls the multicast dispatcher 0x1406a9dc0 (TMulticastScriptDelegate::ProcessMulticastDelegate, see porting/decompilation_notes.md, third pass) on the delegate member with a parameter block holding the value(s) below. The 16-byte MutatorEventData struct is copied into the block in struct order (EventId, EventParam1, EventParam2, EventParam3). The asm (body 0x1419a6bb0) shows the same four dwords. Passing the fields by name matches the copy because the delegate parameter order in the generated macro is the reverse of the block order (see decompilation_notes.md, third pass).
// Declared in generated header: yes
// CONFIDENCE: MEDIUM
MutatorGameplayEvent.Broadcast(MutatorEventData.EventParam3, MutatorEventData.EventParam2, MutatorEventData.EventParam1, MutatorEventData.EventId);
