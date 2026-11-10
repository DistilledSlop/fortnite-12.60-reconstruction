// CONFIDENCE: LOW. Returns the damage this building takes from one event (0 when the hit is ignored). Gates first, then the instigator permission, damage modifiers and the team check, then the health floor for the damage number.
// Original address: 0x14274aba0 (exec thunk). Real body: virtual slot 0xeb8 -> 0x141bfe880 (named BlueprintModifyIncomingDamage in virtual_slots).
if (!bReceivesDamage)                                        // AActor flag byte at +0x5a, bit 0
{
    return 0.0f;
}
if (bDamageSuppressed)                                       // flag byte at +0x3e0, bit 0x2 (name not recovered)
{
    return 0.0f;
}
if (!IncomingDamageFilterQuery.IsEmpty() && IncomingDamageFilterQuery.Matches(InTags))   // +0x398 query, natives 0x143e923e0 / 0x143e933d0
{
    return 0.0f;
}
AFortPlayerController* InstigatorController = Cast<AFortPlayerController>(EventInstigator);   // native 0x142a3d9f0 gives the class
UWorld* World = GetWorld();
if (InstigatorController != nullptr && !bBypassInstigatorCheck)   // bBypassInstigatorCheck = native helper on +0x250 with the settings value (not recovered)
{
    if (!InstigatorController->CanDamageBuilding(this))           // receiver-class virtual 0x1ae0; target not class-resolved (LOW)
    {
        // The native code sends a reason message to the instigator here (subsystem text at world+0x128, else a fixed text), then returns 0.
        return 0.0f;
    }
}
if (UFortDamageSubsystem* Subsystem = GetDamageSubsystem(World))   // native 0x141641300 (world+0x128, class checked)
{
    Damage = Subsystem->ApplyIncomingDamageModifiers(Damage, InTags, this, EventInstigator, DamageCauser);   // native 0x1419519e0
}
if (!bTeamDamageFlag)                                        // flag byte at +0x3e9, bit 0x40 (name not recovered)
{
    if (IsOnSameTeamAsBuilding(EventInstigator) || IsOnSameTeamAsBuilding(DamageCauser))   // native 0x141eff4c0
    {
        return 0.0f;
    }
}
if (World == nullptr)
{
    return Damage;
}
if (OwnerPersistentID == -1 && !bShowDamageNumbersFlag)      // +0x350 is OwnerPersistentID; flag byte at +0x3e5 bit 0x20
{
    return Damage;
}
if (!ShouldDisplayDamageNumbers(DamageNumberOwner))           // virtual 0x748 on this; DamageNumberOwner is the instigator's object at +0x248 when its class matches
{
    return Damage;
}
if (DamageCauser == nullptr || !DamageCauser->IsA(AFortWeapon::StaticClass()) || DamageCauserKind != 14)   // byte at DamageCauser+0x908 must equal 14
{
    return Damage;
}
Damage = FMath::Max(Damage, GetHealth());                    // ABuildingActor::GetHealth (native 0x141b9edb0, via virtual 0x7a8)
SetDamageNumberPending(true);                                // flag byte at +0x3e2, bit 0x20 (name not recovered)
return Damage;
