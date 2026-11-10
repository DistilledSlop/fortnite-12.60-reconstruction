// CONFIDENCE: MEDIUM. Exec thunk: reads InFortPC, InControllingPawn, CustomMovementType (byte) and EffectContainerSpecOnKill (TArray) from the frame (reverse of the inventory order), then calls the native setup helper. The array's element teardown in the thunk is the generated destructor of the TArray and is RAII in the port.
// Original address: 0x142ac2920 (exec). Helper: 0x142450a20 (deps AFortRemoteControlledPawnAthena__SetupRemoteControlPawn__Helper.json).
AFortPlayerController* InFortPC = ReadObjectParam();
AFortPawn* InControllingPawn = ReadObjectParam();
uint8 CustomMovementType = ReadByteParam();
TArray<FFortEffectContainerSpec> EffectContainerSpecOnKill = ReadArrayParam();
SetupRemoteControlPawnHelper(InFortPC, InControllingPawn, CustomMovementType, EffectContainerSpecOnKill);
