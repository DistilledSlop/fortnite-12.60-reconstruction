// Original address: 0x1428c2ec0.
// CONFIDENCE: high
// The reflected exec entry is a virtual-dispatch thunk, not a self-contained body.
// It (1) performs a non-zero-guarded 64-bit increment of the script-frame field at
// +0x20 (FFrame::NextLocalVar): value 0 stays 0, non-zero n becomes n+1, and
// (2) TAIL-CALLS (JMP, not CALL) the class vtable entry at byte offset 0xce0 (index 316).
// The vtable pointer is loaded from [this] (RCX); the dispatch is virtual and must be
// preserved as such (a derived override is not substituted here).
//
// Resolved concrete target: primary vtable 0x146204508 (ctor 0x14265ac60, via GPS
// 0x1428bae30 -> ictor 0x1428bdaa0) slot 0xce0 = 0x142675580. That target is the
// rotate-clockwise body: it sets bIsRotateClockwisePressed (byte 0xf44) = 1, selects the
// rotation axis (from the axis TArray at 0xf70/0xf78 by index 0xf80 when the byte at
// 0x1128 is set, defaulting to the Y/up axis (0,1,0) when the array is empty, else the
// first element), builds a rotation quaternion around that axis scaled by the global
// 0x1478df190 (= 45.0) * 0.5 (a 22.5-degree step) via the helper 0x142c5c060, multiplies
// it into TargetRotationOffset (FQuat at 0xf50), and issues ServerUpdateRotation
// (TargetRotationOffset, bIsRotateClockwisePressed, bIsRotateCounterclockwisePressed)
// through the shared helper 0x1428c04b0. See the evidence JSON for the full chain and the
// outstanding contracts.
*(uint8**)((uint8*)&Stack + 0x20) += (*(uint8**)((uint8*)&Stack + 0x20) != nullptr);
(*(void (**)(void*))(*(uint8**)this + 0xce0))(this);
