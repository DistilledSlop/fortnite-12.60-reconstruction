// Behaviour snippet: FFortAIAttributeReplicationProxy::NetSerialize (body only; spliced inline into the struct header, kind struct_net).
// Original address: 0x1422df0c0
// Decompile source: porting/decompilation_notes.md, Eleventh pass (slot-14 thunk and body, Ghidra probe and asm).
// Thunk note: reached from the ops thunk 0x1427a19e0. Two SerializeIntPacked calls (FArchive slot +0x168).
// The body never writes bOutSuccess (the R9 argument is not used by the asm); it returns true.
// Slot 0x168 is SerializeIntPacked by FArchive declaration order (the call is HIGH, the slot name MEDIUM).
// CONFIDENCE: HIGH
Ar.SerializeIntPacked(Health);
Ar.SerializeIntPacked(MaxHealth);
return true;
