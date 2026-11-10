// CONFIDENCE: MEDIUM. Exec thunk body is native (no params). Resets the simulation state after a hit or tube transition.
// Original address: 0x142aa6370 (exec). Callees: 0x142aa4a00 (ProcessEvent-style call of a reflected function), ForceNetUpdate vslot 0x5e8 (0x143f72550), CollisionComponent vslot 0x618, ProjectileMovementComponent vslot 0x440.
if (Role == ROLE_Authority)                                  // Role byte +0xF0 == 3
{
    ResumeSimulationCount = ResumeSimulationCount + 1;      // int at +0x6F0
}
ProjectileFlags &= ~1;                                        // +0x6F4 bit 0 cleared
bStoppedSimulatingDueToHit = false;                           // +0x654
if (bDisableCollisionOnStop && CollisionComponent != nullptr)  // +0x653 set, +0x3F0
{
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);  // vslot 0x618, arg 3
}
if (ProjectileMovementComponent != nullptr)                   // +0x640
{
    ProjectileMovementComponent->SetUpdatedComponent(CapsuleComponent);   // vslot 0x440 with +0x638
}
ForceNetUpdate();                                             // vslot 0x5E8 -> 0x143f72550
ProcessEvent(FindFunctionChecked(ResumeSimulationFunctionName), nullptr);  // 0x142aa4a00: UObject_FindFunctionChecked on the constant at DAT_1481b8840, then vslot 0x210 with arg 0
