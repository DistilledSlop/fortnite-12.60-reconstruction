// Original address: 0x142757d10.
// CONFIDENCE: high
// The reflected exec entry is a virtual-dispatch thunk, not a self-contained body.
// It (1) performs a non-zero-guarded 64-bit increment of the script-frame field at
// +0x20 (FFrame* passed as the native Parms argument): value 0 stays 0, non-zero n
// becomes n+1, and (2) TAIL-CALLS (JMP, not CALL) the class vtable entry at byte
// offset 0xd18 (index 419). The vtable pointer is loaded from [this] (RCX); the
// dispatch is virtual and must be preserved as such (a derived override is not
// substituted here).
//
// Resolved concrete target: primary vtable 0x146204508 slot 0xd18 = 0x142675c00, which
// copies TargetRotationOffset (16-byte struct at 0xf50) to a local, clears
// bIsRotateCounterclockwisePressed (byte 0xf45), and calls the shared RPC helper
// 0x1428c04b0 with (local 16B, bIsRotateClockwisePressed byte 0xf44, 0). That helper
// builds param struct {TargetRotationOffset@0, bool@16, bool@17}, resolves the UFunction
// via FindFunctionChecked(FName 0x148191eb0) and issues this->ProcessEvent (vtable +0x210)
// = ServerUpdateRotation(ClientRotation, bNewIsRotateCounterclockwisePressed,
// bNewIsRotateClockwisePressed). See the evidence JSON for the full proof chain.
*(uint8**)((uint8*)&Stack + 0x20) += (*(uint8**)((uint8*)&Stack + 0x20) != nullptr);
(*(void (**)(void*))(*(uint8**)this + 0xd18))(this);
