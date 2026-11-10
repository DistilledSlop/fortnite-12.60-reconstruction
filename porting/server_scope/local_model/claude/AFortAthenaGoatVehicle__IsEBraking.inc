// CONFIDENCE: MEDIUM. Original exec 0x1427db430; native body 0x1418f12e0 (inlined). Returns bool through ReturnValue.
// The vehicle returns its movement state block through vtable slot 0xb08 (resolved to AFortAthenaVehicle at 0x14084be00).
// The state flag is bit 1 of the byte at +0x28.
UObject* MovementState = GetVehicleMovementStateBlock();
return (MovementState->NativeRecoveredByte_0x28 >> 1) & 1;
