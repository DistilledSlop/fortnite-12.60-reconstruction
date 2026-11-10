// Behaviour snippet: ABuildingSMActor::MulticastPlayFullHealthEffects (body only; spliced into the generated member function).
// Original address: 0x1427262d0
// Decompile source: captures/9447beeada4d4a84a09c37cdf6afcdef/0x1427262d0.c (asm: .../0x1427262d0.asm)
// Inferred: Standard native NetMulticast-RPC thunk. After the shared native prologue (the counter
//   increment at [rdx+0x20]:  mov rax,[rdx+0x20]; xor r8d,r8d; test rax,rax; setnz r8b; add r8,rax;
//   mov [rdx+0x20],r8) the whole body is a tail-call:
//     mov rax,[rcx]
//     jmp [rax+0xa60]
// i.e. it invokes the class virtual at vtable byte offset 0xa60 with no arguments. The generated header
// (ABuildingSMActor.h lines 666-668) declares, immediately after the RPC, the _Implementation virtual:
//     UFUNCTION(NetMulticast, Unreliable) void MulticastPlayFullHealthEffects();
//     void MulticastPlayFullHealthEffects_Implementation();
// so vtable[0xa60] is MulticastPlayFullHealthEffects_Implementation. This is the canonical UE
// native-RPC-exec -> _Implementation dispatch (same shape as the accepted
// work/q27-11100027/BroadcastOnPullAborted).
// Declared in generated header: yes (ABuildingSMActor.h lines 666-668)
// CONFIDENCE: HIGH
MulticastPlayFullHealthEffects_Implementation();
