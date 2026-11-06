// CONFIDENCE: high; captured native flow and typed dependency behavior.
// Native 0x141663430 calls 0x140680300 directly; that helper returns null.
// Consequently no goals are gathered and the minimum remains MAX_flt.
for (const UFortAIEncounterInfo* Encounter : ActiveEncounters)
{
    if (!bOnlyActiveEncounters || Encounter->EncounterState == EFortEncounterState::Active)
    {
        if (MAX_flt <= Distance * Distance)
        {
            return true;
        }
    }
}
return false;
