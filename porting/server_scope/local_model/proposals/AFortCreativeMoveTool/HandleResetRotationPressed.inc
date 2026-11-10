// Original address: 0x142757c30.
// CONFIDENCE: high
// The reflected exec entry is a virtual-dispatch thunk, not a self-contained body.
// It (1) performs a non-zero-guarded 64-bit increment of the script-frame field at
// +0x20 (FFrame::NextLocalVar): value 0 stays 0, non-zero n becomes n+1, and
// (2) TAIL-CALLS (JMP, not CALL) the class vtable entry at byte offset 0xd30 (index 334).
// The vtable pointer is loaded from [this] (RCX); the dispatch is virtual and must be
// preserved as such (a derived override is not substituted here).
//
// Resolved concrete target: primary vtable 0x146204508 (ctor 0x14265ac60, via GPS
// 0x1428bae30 -> ictor 0x1428bdaa0) slot 0xd30 = 0x142675530. That slot is itself a
// one-instruction virtual forward (JMP [this->vtable + 0xd38]) to slot 0xd38 =
// 0x142680df0, the reset-rotation body, which sets TargetRotationOffset (FQuat at
// 0xf50) to the identity quaternion (global 0x147c31d50 = {0,0,0,1}, with a conditional
// override from the first selected actor when SelectionProperty (0xf32) > 1), clears
// bIsRotateClockwisePressed/bIsRotateCounterclockwisePressed (bytes 0xf44/0xf45), issues
// ServerUpdateRotation(identity, false, false) through the shared helper 0x1428c04b0, and
// issues a trailing dynamic call 0x1406a9dc0(this+0xd88, &identity_quat). See the evidence
// JSON for the full chain and the outstanding contracts.
*(uint8**)((uint8*)&Stack + 0x20) += (*(uint8**)((uint8*)&Stack + 0x20) != nullptr);
(*(void (**)(void*))(*(uint8**)this + 0xd30))(this);
