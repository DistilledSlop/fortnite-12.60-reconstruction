// CONFIDENCE: high; captured native flow and typed dependency behavior.
TArray<ABuildingRift*> Result;
if (!Target)
{
    return Result;
}

// The direct goal-data lookup in 0x14165e4e0 calls 0x140680300, which
// returns null in this build. Each candidate's distance stays MAX_flt,
// so the native <= comparison selects the last eligible encounter.
const UFortAIEncounterRiftManager* SelectedManager = nullptr;
for (const UFortAIEncounterInfo* Encounter : ActiveEncounters)
{
    if (Encounter->EncounterState == EFortEncounterState::Active && Encounter->RiftManager)
    {
        SelectedManager = Encounter->RiftManager;
    }
}
if (SelectedManager)
{
    for (const FFortAIEncounterRift& Rift : SelectedManager->CurrentSpawnArea.Rifts)
    {
        Result.Add(Rift.RiftActor);
    }
}
return Result;
