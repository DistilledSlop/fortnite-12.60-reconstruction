// Original address: 0x1428c2ea0.
// CONFIDENCE: high
// The reflected exec entry is a virtual-dispatch thunk, not a self-contained body.
// It (1) performs a non-zero-guarded 64-bit increment of the script-frame field at
// +0x20 (FFrame::NextLocalVar): value 0 stays 0, non-zero n becomes n+1, and
// (2) TAIL-CALLS (JMP, not CALL) the class vtable entry at byte offset 0xd50 (index 337).
// The vtable pointer is loaded from [this] (RCX); the dispatch is virtual and must be
// preserved as such (a derived override is not substituted here).
//
// Resolved concrete target: primary vtable 0x146204508 (ctor 0x14265ac60, via GPS
// 0x1428bae30 -> ictor 0x1428bdaa0) slot 0xd50 = 0x142675540. That target is the
// reset-scale body: it calls the no-arg helper 0x1428c0140, loads the identity scale
// FVector(1,1,1) from the global at 0x147c31ca0 into a 12-byte stack struct, and issues
// the dynamic call 0x1406a9dc0(this + 0xd78, &FVector(1,1,1)). See the evidence JSON for
// the resolved chain and the outstanding contracts.
*(uint8**)((uint8*)&Stack + 0x20) += (*(uint8**)((uint8*)&Stack + 0x20) != nullptr);
(*(void (**)(void*))(*(uint8**)this + 0xd50))(this);
