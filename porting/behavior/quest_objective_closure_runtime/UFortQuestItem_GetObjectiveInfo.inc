// Native 0x142440da0; validation precedes the definition lookup.
if (!ObjectiveStatHandle.GetRow<FFortQuestObjectiveStatTableRow>(GetName()))
{
    return nullptr;
}
const UFortQuestItemDefinition* Definition = static_cast<const UFortQuestItemDefinition*>(ItemDefinition);
if (Definition)
{
    for (const FFortMcpQuestObjectiveInfo& DefinitionObjective : Definition->Objectives)
    {
        if (ObjectiveStatHandle == DefinitionObjective.ObjectiveStatHandle)
        {
            for (UFortQuestObjectiveInfo* Objective : Objectives)
            {
                if (Objective->BackendName == DefinitionObjective.BackendName)
                {
                    return Objective;
                }
            }
            return nullptr;
        }
    }
}
return nullptr;
