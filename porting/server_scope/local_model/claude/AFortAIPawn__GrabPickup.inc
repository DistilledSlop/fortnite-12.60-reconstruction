// CONFIDENCE: MEDIUM. The pawn grabs a pickup: plays the pickup voice line for an ammo or weapon pickup (in or out of combat), notifies the pickup it was grabbed, and removes it from the unreachable-pickup list.
// Original address: 0x1427a4e40 (exec). Native body: 0x1422d9b90 (pickup definition read 0x1420327f0, combat check 0x1422dca60, voice 0x142356fc0, grab virtual slot 0x640, pickup score 0x14202dc10).
if (Pickup == nullptr || Role != ROLE_Authority)
{
    return;
}

UFortItemDefinition* ItemDefinition = Pickup->GetPickupItemDefinition();
if (ItemDefinition)
{
    const bool bInCombat = IsInCombat();
    if (ItemDefinition->IsA(UFortWeaponItemDefinition::StaticClass()))
    {
        PlayVoiceLine(this, bInCombat ? FName(TEXT("PickedUpWeaponInCombat")) : FName(TEXT("PickedUpWeaponOutOfCombat")), this, 0, -1.0f);
    }
    else if (ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass()))
    {
        PlayVoiceLine(this, bInCombat ? FName(TEXT("PickedUpAmmoInCombat")) : FName(TEXT("PickedUpAmmoOutOfCombat")), this, 0, -1.0f);
    }
}

const FVector GrabOffset = FVector::ZeroVector; // default from the constant pool
const float GrabScore = Pickup->GetRandomizedGrabScore();
Pickup->OnGrabbed(this, GrabScore, GrabOffset, 0); // virtual slot 0x640

UnreachablePickups.Remove(Pickup);
