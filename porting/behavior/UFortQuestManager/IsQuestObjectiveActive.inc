// CONFIDENCE: HIGH; native 0x142447300.
for (const UFortQuestItem* Quest : CurrentQuests)
{
    if (Quest && !Quest->bAllObjectivesComplete)
    {
        for (const UFortQuestObjectiveInfo* Objective : Quest->Objectives)
        {
            if (Objective && Objective->AchievedCount < Objective->RequiredCount && Objective->BackendName == ObjectiveName)
            {
                return true;
            }
        }
    }
}
return false;
