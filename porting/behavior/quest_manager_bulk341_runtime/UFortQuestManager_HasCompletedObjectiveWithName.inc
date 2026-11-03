// CONFIDENCE: HIGH; native 0x1424442e0.
for (UFortQuestItem* Quest : CurrentQuests)
{
    if (Quest && Quest->ItemDefinition == static_cast<const UFortQuestItemDefinition*>(Definition))
    {
        for (const UFortQuestObjectiveInfo* Objective : Quest->Objectives)
        {
            if (Objective->BackendName == BackendName)
            {
                return Objective->RequiredCount <= Objective->AchievedCount;
            }
        }
        return false;
    }
}
return false;
