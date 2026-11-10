// CONFIDENCE: LOW - exec 0x14275eae0 returns the bool from vtable slot 0xf50 (0x141c505a0). The return value is the BL register: true only on the activation path.
// Original addresses: 0x14275eae0 (exec thunk) ; 0x141c505a0 (virtual body, vtable slot 0xf50)
// The blocking-target delegate binding and the owner dispatch use helpers that are declared in deps, not reproduced here.
bool bTriggered = false;
bool bObstructedOut = false;
TArray<AActor*> Targets;
if (!ShouldTrigger(bObstructedOut, Targets))
{
    // Blocking targets: bind OnBlockingBuildingDied on each target (first branch of 0x141c505a0).
    for (AActor* Target : Targets)
    {
        BindOnBlockingBuildingDied(Target);
    }
}
else if (!bTriggerSuppressed)
{
    TArray<AActor*> ValidTargets;
    GetValidTriggerTargets(ValidTargets, /*bForce*/ true);
    bool bBlueprintAllows = false;
    BlueprintShouldTrigger(ValidTargets, bBlueprintAllows);
    if (!bBlueprintAllows)
    {
        DispatchTrapEventViaOwner();
    }
    if (HasAuthority())
    {
        if (AFortPlayerController* OwningController = FindOwningPlayerController())
        {
            SendCombatTrapTriggeredEvent(OwningController, this);
        }
    }
    bTriggered = true;
    return bTriggered;
}
if (ObstructedTargetRecheckInterval > 0.0f && bObstructedOut)
{
    AttemptRetrigger(ObstructedTargetRecheckInterval);
}
return bTriggered;
