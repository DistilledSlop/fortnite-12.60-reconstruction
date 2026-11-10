// CONFIDENCE: HIGH for body; original 0x14234d960. Requires recovered OnRep_IsInAnyStorm.
if (GetLocalRole() == ROLE_Authority && bIsInAnyStorm != bNewIsInAnyStorm)
{
    bIsInAnyStorm = bNewIsInAnyStorm;
    OnRep_IsInAnyStorm();
}
