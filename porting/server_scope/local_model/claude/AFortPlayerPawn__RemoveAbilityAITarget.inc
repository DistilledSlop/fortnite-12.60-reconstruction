// CONFIDENCE: HIGH. Original addresses: exec 0x142a7e440; body 0x142347a60 (read in full, 37 lines). Helpers: 0x142cd4ab0 (FName from a wide string, read in full), 0x1406d80a0 (delegate binding removal, read in full, depth two via 0x142ee6180 and 0x1406bd0b0 not read: described by behaviour only).
// Only the authority removes an AI target. The target's OnDied binding for "AITargetDied" is removed (the wide string is "&AFortPlayerPawn::AITargetDied", so the FName is the text after "::").
if (Role == ROLE_Authority && InTarget)
{
    // Remove the first occurrence of InTarget from AbilityAITargets (a TArray at +0x13f8, count at +0x1400). Equivalent to RemoveAt on the found index; no shrink.
    for (int32 Index = 0; Index < AbilityAITargets.Num(); ++Index)
    {
        if (AbilityAITargets[Index] == InTarget)
        {
            AbilityAITargets.RemoveAt(Index);
            break;
        }
    }
    // 0x1406d80a0: removes the binding (this, "AITargetDied") from the target's InlineMulticastDelegate OnDied (target+0x928).
    InTarget->OnDied.RemoveBinding(this, FName(TEXT("AITargetDied")));
}
