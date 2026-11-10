// CONFIDENCE: LOW
// Original: exec thunk 0x142757f00 calls ABuildingSMActor virtual +0xc70; real body 0x141c18db0. Params: RepairingController (offset 0), ResourcesSpent (offset 8).
// Unresolved: two receivers have undeclared layouts. The component at this+0x270 and the object at RepairingController+0x220 +0x800 are called through vtable slots +0x10 and +0x620 without a declared class, so those calls are not ported.
if (Role != ROLE_Authority)
{
    return;
}
if (!CanBeRepaired())
{
    // Rejected path: notify Blueprint only when the flag, state and target checks all pass (FUN_142753190 is a ProcessEvent of a BP event by FName).
    if (!IsRepairBlockedByFlag() && !IsRepairableStateInvalid() && IsRepairTargetValid())
    {
        BlueprintEventRepairRejected();
    }
    return;
}
UpdateNetDormancyForRepair();
// Maximum health (virtual +0x7b0, ABuildingActor::GetMaxHealth) and missing health (FUN_141ba76d0 = max(0, MaxHealth - current)).
const float MaxHealth = GetMaxHealth();
const float RepairAmount = FMath::Clamp(GetMissingHealth(), 0.0f, MaxHealth);
// TODO(unresolved receivers): repair visual update on the two sub-objects (see notes).
SetRepairHealth(RepairAmount);
PostRepairUpdate();
bUnderRepair = true;
FinishRepairBlueprintEvent();
if (RepairingController)
{
    // Resource refund for the repairer (FUN_1417da1e0 with EFortResourceType looked up by name; not read in full, see notes).
    RefundRepairResources(RepairingController, RepairAmount);
}
