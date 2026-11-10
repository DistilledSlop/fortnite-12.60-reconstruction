// CONFIDENCE: MEDIUM. Exec-thunk-free native Tick override: forwards DeltaSeconds through the pre-tick helper (0x1426862d0) and then the decoration tick (vslot 0x668 -> 0x1426b9c40).
// Original address: 0x142aa7200 (native Tick; the thunk reads DeltaSeconds and calls vslot 0x450 -> 0x1426b6860 which is this same two-step body).
PreTickHelper(DeltaSeconds);      // 0x1426862d0
TickDecoHelper(DeltaSeconds);     // vslot 0x668 -> 0x1426b9c40 (deps AFortProjectileDecoHelper__Tick__TickDecoHelper.json)
