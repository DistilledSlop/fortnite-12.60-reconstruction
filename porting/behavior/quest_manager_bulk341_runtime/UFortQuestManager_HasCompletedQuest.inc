// CONFIDENCE: HIGH; native 0x142444380.
for (UFortQuestItem* Quest : CurrentQuests)
{
    if (Quest && Quest->ItemDefinition == static_cast<const UFortQuestItemDefinition*>(Definition))
    {
        return Quest->quest_state == EFortQuestState::Completed || Quest->quest_state == EFortQuestState::Claimed;
    }
}
return false;
