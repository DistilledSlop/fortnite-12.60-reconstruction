// CONFIDENCE: HIGH; native 0x142441bf0.
for (UFortQuestItem* Quest : CurrentQuests)
{
    if (Quest && Quest->ItemDefinition == static_cast<const UFortQuestItemDefinition*>(Definition))
    {
        return Quest;
    }
}
return nullptr;
