// Original address: 0x1428c2f80.
// CONFIDENCE: high
// The reflected exec entry is a virtual-dispatch thunk, not a self-contained body.
// It (1) performs a non-zero-guarded 64-bit increment of the script-frame field at
// +0x20 (FFrame* passed as the native Parms argument): value 0 stays 0, non-zero n
// becomes n+1, and (2) TAIL-CALLS (JMP, not CALL) the class vtable entry at byte
// offset 0xdd0 (index 448). The vtable pointer is loaded from [this] (RCX); the
// dispatch is virtual and must be preserved as such (a derived override is not
// substituted here).
//
// Resolved concrete target: primary vtable 0x146204508 slot 0xdd0 = 0x142675f30, which
// sets bIsScaleDownPressed (byte 0xf35) = 1 and TAIL-CALLS the shared RPC helper
// 0x1428c0580 with (bIsScaleUpPressed byte 0xf00, 1). That helper builds param struct
// {bool@0, bool@1}, resolves the UFunction via FindFunctionChecked(FName 0x148191ec8) and
// issues this->ProcessEvent (vtable +0x210) = ServerUpdateScalePressed(bNewIsScaleDownPressed,
// bNewIsScaleUpPressed). See the evidence JSON for the full proof chain.
*(uint8**)((uint8*)&Stack + 0x20) += (*(uint8**)((uint8*)&Stack + 0x20) != nullptr);
(*(void (**)(void*))(*(uint8**)this + 0xdd0))(this);
