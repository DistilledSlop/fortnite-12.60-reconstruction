// Behaviour snippet: ABuildingGameplayActorBlade::BroadcastOnPullStart (body only; spliced into the generated member function).
// Original address: 0x1427261d0
// Decompile source: captures/69d216f1f5fb4333b1a8d5cd83cb4996/0x1427261d0.c (asm: .../0x1427261d0.asm)
// Inferred: Standard native NetMulticast-RPC thunk. After the shared native prologue the body is a
//   tail-call:  mov rax,[rcx];  jmp [rax+0xa98]
// i.e. it invokes the class virtual at vtable byte offset 0xa98 with no arguments. The generated header
// (ABuildingGameplayActorBlade.h) declares the only two non-UFUNCTION virtuals of the class as
//   BroadcastOnPullAborted_Implementation()  and  BroadcastOnPullStart_Implementation();
// the two adjacent vtable slots 0xa90/0xa98 map to them in declaration order, so 0xa98 is
// BroadcastOnPullStart_Implementation. This is the canonical UE RPC-exec -> _Implementation dispatch.
// Declared in generated header: yes (ABuildingGameplayActorBlade.h lines 21-23:
//   UFUNCTION(NetMulticast, Reliable) void BroadcastOnPullStart(); + void BroadcastOnPullStart_Implementation();)
// CONFIDENCE: HIGH
BroadcastOnPullStart_Implementation();
