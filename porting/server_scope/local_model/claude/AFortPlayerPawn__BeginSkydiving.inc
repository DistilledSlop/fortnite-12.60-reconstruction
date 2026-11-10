// CONFIDENCE: LOW
// Original address: 0x142a7aee0 (exec thunk). Reads bFromBus (bool). Calls thunk_FUN_140c31764(this, bFromBus), which calls FUN_1495a4a24(...).
// A later original-client runtime image reveals a virtual dispatch at slot 0x1228; the Athena target updates skydiving state and may broadcast an event.
// This call is a proposal; the implementation and remaining helper effects are not yet integrated or verified. See the dependency evidence.
BeginSkydivingInternal(bFromBus);
