// CONFIDENCE: LOW
// Original address: 0x142a7aee0 (exec thunk). Reads bFromBus (bool). Calls thunk_FUN_140c31764(this, bFromBus), which calls FUN_1495a4a24(...).
// FUN_1495a4a24 does not decode as a function body: captures/*/0x1495a4a24.asm shows an OR.LOCK into an unresolved base register, then RETF.
// Its effect is therefore not recovered. The ported body keeps the call and the argument. See deps/AFortPlayerPawn__BeginSkydiving__FUN_1495a4a24.json.
BeginSkydivingInternal(bFromBus);
