// CONFIDENCE: LOW. Original exec 0x1427d8600; native body 0x1418fb480 (inlined). Parameter: Pawn (UObject*).
// For every spawn entry (array at +0x8a0, count +0x8a8, 20-byte entries) the item definition is resolved and checked as a
// UFortWorldItemDefinition. The spawn origin is the root component's relative location, or the default vector without a root.
// The original draws a random unit-sphere sample per entry and consumes rand() for it. The sample is not used afterwards,
// so the loop only reproduces the RNG consumption. A callback object carrying the item, count, pawn and origin + 20 height is
// queued through ScheduleDelayedCall.
for (const FSupplyDropSpawnEntry& Entry : NativeRecoveredSpawnEntries_0x8a0)
{
    UFortWorldItemDefinition* ItemDefinition = ResolveWorldItemDefinition(Entry);

    const FVector Origin = RootComponent ? RootComponent->RelativeLocation : FVector(0.0f, 0.0f, 0.0f);

    do
    {
        float LengthSquared;
        do
        {
            const float X = (float)FMath::Rand() * 6.103702e-05f - 1.0f;
            const float Y = (float)FMath::Rand() * 6.103702e-05f - 1.0f;
            const float Z = (float)FMath::Rand() * 6.103702e-05f - 1.0f;
            LengthSquared = X * X + Y * Y + Z * Z;
        } while (LengthSquared > 1.0f);
    } while (LengthSquared < 0.0001f);

    FSupplyDropSpawnCallback* Callback = new FSupplyDropSpawnCallback();
    Callback->Owner = this;
    Callback->ItemDefinition = ItemDefinition;
    Callback->NativeRecoveredInt32_0x10 = Entry.Count;
    Callback->Pawn = Pawn;
    Callback->Origin = Origin;
    Callback->Origin.Z = Origin.Z + 20.0f;

    ScheduleDelayedCall(this, Callback);
}
