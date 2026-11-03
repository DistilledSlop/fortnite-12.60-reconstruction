// CONFIDENCE: HIGH; native 0x1424472c0.
for (UFortQuestItem* Quest : CurrentQuests)
{
    if (Quest && Quest->ItemDefinition == static_cast<const UFortQuestItemDefinition*>(Definition))
    {
        return Quest->quest_state == EFortQuestState::Active;
    }
}
return false;
