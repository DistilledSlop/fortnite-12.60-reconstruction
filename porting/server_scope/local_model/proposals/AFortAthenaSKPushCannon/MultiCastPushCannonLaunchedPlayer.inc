// Behaviour snippet: AFortAthenaSKPushCannon::MultiCastPushCannonLaunchedPlayer (body only; spliced into the generated member function).
// Original address: 0x1428315e0
// Decompile source: captures/bb4acc6161154cdbad8c73096e883910/0x1428315e0.c (asm: .../0x1428315e0.asm)
// Inferred: Standard native NetMulticast-RPC thunk. After the shared native prologue (P_FINISH at [rdx+0x20]) the
//   body is a tail-call:  mov rax,[rcx];  jmp [rax+0xeb8]
//   i.e. it invokes the class virtual at vtable byte offset 0xeb8 with no arguments. The generated header
//   (AFortAthenaSKPushCannon.h) declares exactly two own non-UFUNCTION virtuals, in declaration order
//   MultiCastPushCannonLaunchedPlayer_Implementation() (line 63) and ServerSetPassengerTorquing_Implementation
//   (line 67); GetLifetimeReplicatedProps (line 69) is an override of a base slot and adds none. The sibling
//   Server-RPC exec (0x142831990) confirms the layout: it calls vtable slot 0xec0 (validate, on failure the
//   string at 0x14632c860 through the shared RPC_ValidateFailed dispatcher 0x142d6e0f0) and slot 0xec8
//   (implementation), so 0xeb8 is the first own slot: MultiCastPushCannonLaunchedPlayer_Implementation.
//   This is the canonical UE RPC-exec -> _Implementation dispatch.
// Declared in generated header: yes (AFortAthenaSKPushCannon.h lines 61-63:
//   UFUNCTION(NetMulticast, Reliable) void MultiCastPushCannonLaunchedPlayer(); + void MultiCastPushCannonLaunchedPlayer_Implementation();)
// CONFIDENCE: HIGH
MultiCastPushCannonLaunchedPlayer_Implementation();
