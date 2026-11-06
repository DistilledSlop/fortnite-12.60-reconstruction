// CONFIDENCE: high; complete native source binding; see HANDOFF.md.
// Native 0x1424511c0; see HANDOFF.md.
if (!Quest || !Quest->ItemDefinition) return false;
const UFortQuestItemDefinition* Definition = static_cast<UFortQuestItemDefinition*>(Quest->ItemDefinition);
// Original assumes ItemDefinition is a quest definition and does not cast-check.
if (Definition->bShouldDisplayOverallQuestInformation) {
    int32 Required = Definition->ObjectiveCompletionCount;
    if (Required < 1) Required = Definition->Objectives.Num();
    return Quest->LastNotifiedQuestCount != Required &&
        Objective->AchievedCount >= Objective->RequiredCount;
}
const int32 Achieved = Objective->AchievedCount;
if (Objective->LastNotifiedCount == Achieved) return false;
switch (DefinitionObjective->DynamicStatusUpdateType) {
case EObjectiveStatusUpdateType::Always: return true;
case EObjectiveStatusUpdateType::OnComplete: return Achieved >= Objective->RequiredCount;
case EObjectiveStatusUpdateType::OnPercent: {
    if (Achieved >= Objective->RequiredCount) return true;
    // x86 IMUL wraps the 32-bit product before signed division by 100.
    const int32 Product = static_cast<int32>(static_cast<uint32>(DefinitionObjective->DynamicStatusUpdatePercentInterval) * static_cast<uint32>(Objective->RequiredCount));
    const int32 Step = Product / 100;
    if (Step == 0) return false;
    const int32 PreviousBucket = Objective->LastNotifiedCount / Step;
    const int32 CurrentBucket = Achieved / Step;
    return CurrentBucket > PreviousBucket;
}
default: return false;
}
