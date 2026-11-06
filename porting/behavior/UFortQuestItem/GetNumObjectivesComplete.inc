// CONFIDENCE: HIGH
int32 Count = 0;
for (const UFortQuestObjectiveInfo* Objective : Objectives)
{
    if (Objective->bVisible && Objective->AchievedCount >= Objective->RequiredCount)
    {
        ++Count;
    }
}
return Count;
