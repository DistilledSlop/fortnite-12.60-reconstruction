// CONFIDENCE: high; captured native fields and complete branches.
// Native 0x1424471e0: only active quests and the first matching objective qualify.
if (quest_state == EFortQuestState::Active)
{
    for (const UFortQuestObjectiveInfo* Objective : Objectives)
    {
        if (Objective->BackendName == BackendName)
        {
            return Objective->bActive && Objective->AchievedCount < Objective->RequiredCount;
        }
    }
}
return false;
