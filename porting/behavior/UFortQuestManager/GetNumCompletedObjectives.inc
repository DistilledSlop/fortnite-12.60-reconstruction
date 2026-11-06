// CONFIDENCE: HIGH; native 0x142440ad0.
for (UFortQuestItem* Quest : CurrentQuests)
{
    if (Quest && Quest->ItemDefinition == static_cast<const UFortQuestItemDefinition*>(Definition))
    {
        int32 Count = 0;
        for (const UFortQuestObjectiveInfo* Objective : Quest->Objectives)
        {
            if (Objective && Objective->RequiredCount <= Objective->AchievedCount)
            {
                ++Count;
            }
        }
        return Count;
    }
}
return 0;
