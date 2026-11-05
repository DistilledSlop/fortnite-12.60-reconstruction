// Native 0x142447110.
if (quest_state != EFortQuestState::Active)
{
    return false;
}
const UFortQuestObjectiveInfo* Objective = GetObjectiveInfo(ObjectiveStatHandle);
return Objective && Objective->bActive && Objective->AchievedCount < Objective->RequiredCount;
