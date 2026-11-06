// CONFIDENCE: HIGH; native 0x14243ddf0.
OutTotalAchievedCount = 0;
OutTotalRequiredCount = 0;
OutLastKnownAchievedCount = 0;
const int32 CompletionCount = static_cast<const UFortQuestItemDefinition*>(ItemDefinition)->ObjectiveCompletionCount;
if (CompletionCount < 1)
{
    OutTotalRequiredCount = 0;
    OutTotalAchievedCount = 0;
    for (const UFortQuestObjectiveInfo* Objective : Objectives)
    {
        if (Objective->bVisible)
        {
            OutTotalAchievedCount = static_cast<int32>(static_cast<uint32>(OutTotalAchievedCount) + static_cast<uint32>(Objective->AchievedCount));
            OutTotalRequiredCount = static_cast<int32>(static_cast<uint32>(OutTotalRequiredCount) + static_cast<uint32>(Objective->RequiredCount));
            OutLastKnownAchievedCount = static_cast<int32>(static_cast<uint32>(OutLastKnownAchievedCount) + static_cast<uint32>(UseLastKnownMcp ? Objective->LastKnownMCPCount : Objective->LastNotifiedCount));
        }
    }
}
else
{
    for (const UFortQuestObjectiveInfo* Objective : Objectives)
    {
        if (Objective->bVisible)
        {
            if (Objective->RequiredCount <= Objective->AchievedCount)
            {
                OutTotalAchievedCount = static_cast<int32>(static_cast<uint32>(OutTotalAchievedCount) + 1u);
            }
            if (Objective->RequiredCount <= (UseLastKnownMcp ? Objective->LastKnownMCPCount : Objective->LastNotifiedCount))
            {
                OutLastKnownAchievedCount = static_cast<int32>(static_cast<uint32>(OutLastKnownAchievedCount) + 1u);
            }
        }
    }
    OutTotalRequiredCount = CompletionCount;
}
