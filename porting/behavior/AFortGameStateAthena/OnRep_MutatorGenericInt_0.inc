// Behaviour snippet: AFortGameStateAthena::OnRep_MutatorGenericInt_0 (body only; spliced into the generated member function).
// Original address: 0x142920570
// Decompile source: porting/generated/decomp_improved/AFortGameStateAthena/OnRep_MutatorGenericInt_0.c
// Proxy score: 73.6 (portability.json)
// Inferred: Decompile calls the multicast dispatcher 0x1406a9dc0 (TMulticastScriptDelegate::ProcessMulticastDelegate, see porting/decompilation_notes.md, third pass) on the delegate member with a parameter block holding the value(s) below. The block holds the constant 0 at offset 0 and MutatorGenericInt_0 at offset 4 (asm of the body 0x1419a6c10: MOV [RSP+0x30],0; MOV [RSP+0x34],field). Passed by name, the value goes to NewIntegerValue and the index 0 to GenericIntegerIndex, because the generated macro order is the reverse of the block order.
// Declared in generated header: yes
// CONFIDENCE: MEDIUM
MutatorGenericIntegerUpdated.Broadcast(MutatorGenericInt_0, 0);
