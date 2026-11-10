// CONFIDENCE: HIGH that the shipped body has no side effects. Original exec 0x142a4aab0 (RPC thunk, one struct parameter CreateBuildingData, copied by value into a stack block).
// Validate is virtual slot 0x1178 -> Stub_ReturnTrue (0x1406801d0), so no validation runs.
// Implementation is virtual slot 0x1180 -> 0x1423aad00 (20 lines). Its body reads GetWorld()->GameState, checks IsA(AFortGameStateZone), and reads the first int of CreateBuildingData against the GameState range field (+0xcf0). Both the success and the failure path end in return, so the body performs no writes.
// The struct is passed by pointer to the implementation; only its first int is read. Nothing else is ported because nothing else is done.
(void)CreateBuildingData;
