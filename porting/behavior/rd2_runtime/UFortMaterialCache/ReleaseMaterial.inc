// Behaviour snippet: UFortMaterialCache::ReleaseMaterial (body only; spliced into the generated member function).
// Original address: 0x141f44b40 (thunk 0x1429972c0). Asm: porting/generated/native-cap-direct/0x141f44b40.asm
// Offset evidence: Reservations = TArray<FMaterialReservation> at +0x28 (Num at +0x30), element stride 0x10.
// Offset evidence: FMaterialReservation.MaterialInstance at element +0x8; the in-use byte at element +0x0 is unreflected in FMaterialReservation and is written through a raw byte pointer.
// Verbosity-gated diagnostic log at 0x142c54b70 (guarded by byte [0x1481348e0] >= 2, reached only when no entry matches) elided (no state change)
// CONFIDENCE: MEDIUM
if (MaterialInstance == nullptr)
{
    return;
}
for (int32 Index = 0; Index < Reservations.Num(); ++Index)
{
    FMaterialReservation& Entry = Reservations[Index];
    if (Entry.MaterialInstance == MaterialInstance)
    {
        reinterpret_cast<uint8*>(&Entry)[0] = 0;
        MaterialInstance = nullptr;
        return;
    }
}
