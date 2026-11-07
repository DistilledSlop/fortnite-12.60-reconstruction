// Behaviour snippet: UFortMaterialCache::ReserveMaterial (body only; spliced into the generated member function).
// Original address: 0x141f45990 (thunk 0x1429973d0). Asm: porting/generated/native-cap-direct/0x141f45990.asm
// Offset evidence: Reservations = TArray<FMaterialReservation> at +0x28 (Num at +0x30), element stride 0x10.
// Offset evidence: FMaterialReservation.MaterialInstance at element +0x8; in-use byte at element +0x0 (unreflected) is tested and set through a raw byte pointer.
// CONFIDENCE: HIGH
OutMaterialInstance = nullptr;
for (int32 Index = 0; Index < Reservations.Num(); ++Index)
{
    FMaterialReservation& Entry = Reservations[Index];
    uint8* InUse = reinterpret_cast<uint8*>(&Entry);
    if (*InUse == 0)
    {
        OutMaterialInstance = Entry.MaterialInstance;
        *InUse = 1;
        break;
    }
}
return OutMaterialInstance != nullptr;
