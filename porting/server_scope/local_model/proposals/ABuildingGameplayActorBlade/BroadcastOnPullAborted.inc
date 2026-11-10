// Behaviour snippet: ABuildingGameplayActorBlade::BroadcastOnPullAborted (body only; spliced into the generated member function).
// Original address: 0x142726190
// Decompile source: captures/69d216f1f5fb4333b1a8d5cd83cb4996/0x142726190.c (asm: .../0x142726190.asm)
// Inferred: Standard native NetMulticast-RPC thunk. After the shared native prologue the body is a
//   tail-call:  mov rax,[rcx];  jmp [rax+0xa90]
// i.e. it invokes the class virtual at vtable byte offset 0xa90 with no arguments. The generated header
// (ABuildingGameplayActorBlade.h) declares the only two non-UFUNCTION virtuals of the class as
//   BroadcastOnPullAborted_Implementation()  and  BroadcastOnPullStart_Implementation();
// the two adjacent vtable slots 0xa90/0xa98 map to them in declaration order, so 0xa90 is
// BroadcastOnPullAborted_Implementation. This is the canonical UE RPC-exec -> _Implementation dispatch.
// Declared in generated header: yes (ABuildingGameplayActorBlade.h lines 17-19:
//   UFUNCTION(NetMulticast, Reliable) void BroadcastOnPullAborted(); + void BroadcastOnPullAborted_Implementation();)
// CONFIDENCE: HIGH
BroadcastOnPullAborted_Implementation();
