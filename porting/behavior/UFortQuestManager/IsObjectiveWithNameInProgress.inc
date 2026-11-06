// CONFIDENCE: HIGH; native 0x142447260.
for (UFortQuestItem* Quest : CurrentQuests)
{
    if (Quest && Quest->ItemDefinition == static_cast<const UFortQuestItemDefinition*>(Definition))
    {
        if (Quest->quest_state == EFortQuestState::Active)
        {
            for (const UFortQuestObjectiveInfo* Objective : Quest->Objectives)
            {
                if (Objective->BackendName == BackendName)
                {
                    return Objective->bActive && Objective->AchievedCount < Objective->RequiredCount;
                }
            }
        }
        return false;
    }
}
return false;
