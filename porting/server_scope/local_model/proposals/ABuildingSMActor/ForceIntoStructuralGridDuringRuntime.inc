// Behaviour snippet: ABuildingSMActor::ForceIntoStructuralGridDuringRuntime (body only; spliced into the generated member function).
// Original address: 0x142725810
// Decompile source: captures/e6d09d88f2d1409d99e93c27c1388f16/0x142725810.c (asm: .../0x142725810.asm)
// Inferred: Standard native NetMulticast-RPC thunk. After the shared native prologue (the
//   [rdx+0x20] "flag touch" increment) the body is a tail-call:
//     mov rax,[rcx];  jmp [rax+0xa58]
//   i.e. it invokes the class virtual at vtable byte offset 0xa58 (index 331) with no arguments.
// The generated header (ABuildingSMActor.h line 629-630) declares the UFUNCTION and its
//   _Implementation virtual:
//     UFUNCTION(NetMulticast, Reliable) void ForceIntoStructuralGridDuringRuntime();
//     void ForceIntoStructuralGridDuringRuntime_Implementation();
//   ForceIntoStructuralGridDuringRuntime_Implementation is the FIRST new (non-override) virtual
//   declared by ABuildingSMActor, so its slot is the base offset where the class's new vtable slots
//   begin: index 331 = byte offset 0xa58. This matches the exec tail-call exactly.
// This is the canonical UE RPC-exec -> _Implementation dispatch.
// Declared in generated header: yes (ABuildingSMActor.h lines 628-630).
// CONFIDENCE: HIGH
ForceIntoStructuralGridDuringRuntime_Implementation();
