// CONFIDENCE: MEDIUM; thunk 0x142a7fbb0: validate (vcall 0xfc8) is the return-true stub 0x1406801d0. Impl 0x14234ad30 (1) copies the 0x28-byte incoming input into a snapshot at this+0x1194 via FUN_14192a490 (dwords 0-3,5,6, qword 7, dword 9; index 4 and 8 skipped, LOW), (2) copies the full input into VehicleInputStateUnreliable (this+0x11c0, five qwords), (3) forwards the Timestamp float (XMM1) to the UFunction FName DAT_1481b4958 via FUN_142a755c0 (ProcessEvent with a 4-byte payload; name unresolved).
CrouchSnapshotBlock_0x1194 = ReliableInput;   // FUN_14192a490, dwords 0-3,5,6, qword 7, dword 9
VehicleInputStateUnreliable = ReliableInput;   // full 0x28-byte copy
ProcessEventVehicleInputStateUnreliable(Timestamp);
