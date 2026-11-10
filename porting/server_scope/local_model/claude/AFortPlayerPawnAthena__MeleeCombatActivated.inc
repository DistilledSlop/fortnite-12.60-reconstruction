// CONFIDENCE: HIGH
// Original: exec thunk 0x142a837b0; body is inline in the thunk (no RPC). Helpers:
//   IsMeleeCombatAllowed()          = virtual slot 0x6b0, target 0x144430060 (dep: ...__IsMeleeCombatAllowed.json)
//   GetCurrentWeaponContext()       = virtual slot 0x150, target 0x143f793a0 (dep: ...__GetCurrentWeaponContext.json)
//   ExtendMeleeSlowRemaining(v)     = native 0x1419bfa40 (dep: ...__ExtendMeleeSlowRemaining.json)
// The thunk consumes no parameters.
if (IsMeleeCombatAllowed())
{
    // Extend the slow-motion timer owned by the object at this+0x280 (gap offset 0x8 + 0x278) to at least MeleeCombatSlowDuration.
    ExtendMeleeSlowRemaining(MeleeCombatSlowDuration);
    if (CurrentWeapon != nullptr)
    {
        // Copy the weapon-context value (ctx+0x540) into the weapon's cached field (+0xa40).
        CurrentWeapon->CachedMeleeCombatValue = GetCurrentWeaponContext()->CachedValue540;
    }
}
