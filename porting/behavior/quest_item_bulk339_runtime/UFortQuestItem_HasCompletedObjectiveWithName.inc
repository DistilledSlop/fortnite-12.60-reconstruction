// CONFIDENCE: high; captured native fields and complete branches.
// Native 0x142444280: first matching backend name controls the result.
for (const UFortQuestObjectiveInfo* Objective : Objectives)
{
    if (Objective->BackendName == BackendName)
    {
        return Objective->RequiredCount <= Objective->AchievedCount;
    }
}
return false;
