// Original address: 0x1428c4900.
// CONFIDENCE: high
// Reflected Server RPC (flags 0x80280cc0 = Net|NetReliable|NetServer), no reflected params.
// The exec entry is the standard Server-RPC prologue plus a validate/body virtual-dispatch pair:
//   (1) Non-zero-guarded 64-bit increment of the script-frame field at +0x20 (FFrame::NextLocalVar):
//       value 0 stays 0, non-zero n becomes n+1.
//   (2) CALL the class vtable entry at byte offset 0xc18 (index 774) — the per-RPC _Validate thunk.
//       The resolved target (primary vtable 0x146204508 slot 0xc18 = 0x1406801d0) is the trivial
//       `return 1;` no-param validator (the SAME shared validator as ServerHandshake), so validation
//       always succeeds here.
//   (3) If the validator returns false, call RPC_ValidateFailed(TEXT("ServerResetScale_Validate"))
//       and return (the string address is loaded via LEA RIP+disp to the wide literal in the image).
//   (4) Otherwise TAIL-CALL (JMP, not CALL) the class vtable entry at byte offset 0xc20 (index 775)
//       — the Server-RPC body. The vtable pointer is loaded from [this] (RBX/RCX); the dispatch is
//       virtual and must be preserved as such (a derived override is not substituted here).
//
// Resolved concrete targets (primary vtable 0x146204508): validate slot 0xc18 = 0x1406801d0 (returns 1);
// body slot 0xc20 = 0x1426818f0. See the evidence JSON for the full proof chain.
*(uint8**)((uint8*)&Stack + 0x20) += (*(uint8**)((uint8*)&Stack + 0x20) != nullptr);
if (!(*(bool (**)(void*))(*(uint8**)this + 0xc18))(this))
{
  RPC_ValidateFailed(TEXT("ServerResetScale_Validate"));
  return;
}
(*(void (**)(void*))(*(uint8**)this + 0xc20))(this);
