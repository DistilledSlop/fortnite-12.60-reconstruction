// CONFIDENCE: high; native capture and reflected fields reviewed.
// Native 0x142441050. The stored definition is assumed to be a quest definition.
const UFortQuestItemDefinition* Definition = KnownQuestDef
    ? KnownQuestDef : static_cast<const UFortQuestItemDefinition*>(ItemDefinition);
if (!Definition)
{
    return 0.0f;
}
if (quest_state == EFortQuestState::Completed)
{
    return 100.0f;
}
if (quest_state != EFortQuestState::Active)
{
    return 0.0f;
}

float Progress = 0.0f;
int32 Denominator = Definition->ObjectiveCompletionCount;
if (Denominator >= 0)
{
    for (const UFortQuestObjectiveInfo* Objective : Objectives)
    {
        if (Objective->bVisible && Objective->AchievedCount >= Objective->RequiredCount)
        {
            Progress += 1.0f;
        }
    }
}
else
{
    for (const UFortQuestObjectiveInfo* Objective : Objectives)
    {
        if (Objective->bVisible)
        {
            Progress += static_cast<float>(Objective->AchievedCount) / static_cast<float>(Objective->RequiredCount);
        }
    }
    Denominator = Objectives.Num();
}
return Denominator > 0 ? (Progress * 100.0f) / static_cast<float>(Denominator) : 0.0f;
