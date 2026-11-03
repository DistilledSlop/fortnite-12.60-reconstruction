// CONFIDENCE: HIGH; native 0x14243f700 and stable-sort helper 0x142431be0.
OutCompletedQuests.Reset();
for (UFortQuestItem* Quest : CurrentQuests)
{
    if (Quest && Quest->quest_state == EFortQuestState::Completed)
    {
        OutCompletedQuests.Add(Quest);
    }
}
Algo::StableSort(OutCompletedQuests, [](const UFortQuestItem* A, const UFortQuestItem* B)
{
    const UFortQuestItemDefinition* ADefinition = static_cast<const UFortQuestItemDefinition*>(A->ItemDefinition);
    const UFortQuestItemDefinition* BDefinition = static_cast<const UFortQuestItemDefinition*>(B->ItemDefinition);
    return ADefinition && BDefinition && ADefinition->ClaimPriority < BDefinition->ClaimPriority;
});
