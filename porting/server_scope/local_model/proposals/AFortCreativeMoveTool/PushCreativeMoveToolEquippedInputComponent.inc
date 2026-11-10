// Original address: 0x1428c3e80.
// CONFIDENCE: high
// The reflected exec entry is a virtual-dispatch thunk, not a self-contained body.
// It (1) performs a non-zero-guarded 64-bit increment of the script-frame field at
// +0x20 (FFrame::NextLocalVar): value 0 stays 0, non-zero n becomes n+1, and
// (2) TAIL-CALLS (JMP, not CALL) the class vtable entry at byte offset 0xc70 (index 784).
// The vtable pointer is loaded from [this] (RCX); the dispatch is virtual and must be
// preserved as such (a derived override is not substituted here).
//
// Resolved concrete target: primary vtable 0x146204508 (ctor 0x14265ac60, via GPS
// 0x1428bae30 -> ictor 0x1428bdaa0) slot 0xc70 = 0x14267f790. See the evidence JSON
// for the full proof chain.
*(uint8**)((uint8*)&Stack + 0x20) += (*(uint8**)((uint8*)&Stack + 0x20) != nullptr);
(*(void (**)(void*))(*(uint8**)this + 0xc70))(this);
