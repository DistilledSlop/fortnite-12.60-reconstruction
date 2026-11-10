// Original address: 0x1428c2e20.
// CONFIDENCE: high
// The reflected exec entry is a virtual-dispatch thunk, not a self-contained body.
// It (1) performs a non-zero-guarded 64-bit increment of the script-frame field at
// +0x20 (FFrame::NextLocalVar): value 0 stays 0, non-zero n becomes n+1, and
// (2) TAIL-CALLS (JMP, not CALL) the class vtable entry at byte offset 0xd70 (index 365).
// The vtable pointer is loaded from [this] (RCX); the dispatch is virtual and must be
// preserved as such (a derived override is not substituted here).
//
// Resolved concrete target: primary vtable 0x146204508 (ctor 0x14265ac60, via GPS
// 0x1428bae30 -> ictor 0x1428bdaa0) slot 0xd70 = 0x1426754b0, which sets
// bIsPullPressed (byte 0xf37, reflected bool bit) and issues the Server RPC
// ServerUpdateTractorBeam(NativeRecoveredFloat_0xf3c, bIsPushPressed, true) through the
// shared helper 0x1428c05d0 (FindFunctionChecked(FName 0x148191ed0) + ProcessEvent,
// param struct {float@0, bool@4, bool@5} = (NewTargetTractorBeamOffset, bNewIsPushPressed,
// bNewIsPullPressed)). See the evidence JSON for the full proof chain.
*(uint8**)((uint8*)&Stack + 0x20) += (*(uint8**)((uint8*)&Stack + 0x20) != nullptr);
(*(void (**)(void*))(*(uint8**)this + 0xd70))(this);
