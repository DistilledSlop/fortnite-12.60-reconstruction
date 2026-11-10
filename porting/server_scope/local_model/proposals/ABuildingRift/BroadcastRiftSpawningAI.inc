// Behaviour snippet: ABuildingRift::BroadcastRiftSpawningAI (body only; spliced into the generated member function).
// Original address: 0x14274fd50
// Decompile source: captures/69d216f1f5fb4333b1a8d5cd83cb4996/0x14274fd50.c (asm: .../0x14274fd50.asm)
// Inferred: Standard native NetMulticast-RPC thunk. After the shared native prologue (the
//   [rdx+0x20] "flag touch" increment) the body is a tail-call:
//     mov rax,[rcx];  jmp [rax+0xa00]
//   i.e. it invokes the class virtual at vtable byte offset 0xa00 (index 320) with no arguments.
// The generated header (ABuildingRift.h) declares exactly ONE non-UFUNCTION virtual for this
//   class:  void BroadcastRiftSpawningAI_Implementation();
//   (GetLifetimeReplicatedProps is an override of an inherited slot, not a new slot.) The
//   parent ABuildingActor's vtable has 320 entries (indices 0..319), so ABuildingRift's first and
//   only new slot, index 320 = byte offset 0xa00, is BroadcastRiftSpawningAI_Implementation.
// This is the canonical UE RPC-exec -> _Implementation dispatch.
// Declared in generated header: yes (ABuildingRift.h lines 207-209:
//   UFUNCTION(NetMulticast, Unreliable) void BroadcastRiftSpawningAI();
//   void BroadcastRiftSpawningAI_Implementation();)
// CONFIDENCE: HIGH
BroadcastRiftSpawningAI_Implementation();
