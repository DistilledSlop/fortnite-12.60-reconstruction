// CONFIDENCE: HIGH for control flow; LOW for the meaning of the two stored byte copies. Original addresses: _Implementation 0x142334ea0 (read in full, 29 lines); byte copy helper 0x1419297f0 (read in full: it copies one byte).
// Autonomous proxies skip the update (Role at pawn+0xf0; ROLE_AutonomousProxy = 2).
if (Role != ROLE_AutonomousProxy)
{
    FFortAthenaVehicleInputStateReliable InputCopy = ReliableInput;
    AActor* Vehicle = VehicleStateLocal.Vehicle;
    if (Vehicle && Vehicle->IsA(AFortAthenaVehicle::StaticClass())) // 0x1428381e0 (named), class-hierarchy test read in full
    {
        Vehicle->ApplyVehicleInputStateReliable(InputCopy);           // vtable 0x850; AFortAthenaVehicle entry is a STUB (0x1406801c0)
    }
    NativeRecoveredByte_CrouchMeshOffset4 = InputCopy.InputBits;     // byte copy 0x1419297f0 into this->CrouchMeshOffset+4
    VehicleInputStateReliable[0] = InputCopy.InputBits;
}
