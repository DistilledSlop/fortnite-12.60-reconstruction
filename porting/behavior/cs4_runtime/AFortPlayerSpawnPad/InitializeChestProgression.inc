// Behaviour snippet: AFortPlayerSpawnPad::InitializeChestProgression (body only; spliced into the generated member function).
// Original body: 0x1424b1340 (thunk 0x142a892a0).
// Evidence: Chests = TArray<UStaticMeshComponent*> at +0xb30 (Num +0xb38). Unreflected TArray of 12-byte {double@0, int32@8} elements at +0xb40 (Data, NativeRecoveredPtr_0xb40), Num +0xb48, Max +0xb4c. Chest pointee reads +0x11c (double) and +0x124 (int32).
// CONFIDENCE: MEDIUM
NativeRecoveredInt32_0xb48 = 0;
if (NativeRecoveredInt32_0xb4c != 0)
{
    AFortCheckPointSplineActor::NativeArraySetCapacity140694750(&NativeRecoveredPtr_0xb40, 0);
}
static_assert(sizeof(AFortCheckPointSplineActor) > 0, "AFortCheckPointSplineActor must be complete for the helper calls");
for (int32 ChestIndex = 0; ChestIndex < Chests.Num(); ++ChestIndex)
{
    const uint8* Chest = reinterpret_cast<const uint8*>(Chests[ChestIndex]);
    const double ChestValue = *reinterpret_cast<const double*>(Chest + 0x11c);
    const int32 ChestTag = *reinterpret_cast<const int32*>(Chest + 0x124);
    const int32 OldNum = NativeRecoveredInt32_0xb48;
    NativeRecoveredInt32_0xb48 = OldNum + 1;
    if (NativeRecoveredInt32_0xb48 > NativeRecoveredInt32_0xb4c)
    {
        AFortCheckPointSplineActor::NativeArrayGrowSlack12140694640(&NativeRecoveredPtr_0xb40);
    }
    uint8* Elem = reinterpret_cast<uint8*>(NativeRecoveredPtr_0xb40) + static_cast<int64>(OldNum) * 12;
    *reinterpret_cast<double*>(Elem) = ChestValue;
    *reinterpret_cast<int32*>(Elem + 8) = ChestTag;
}
