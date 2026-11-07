// Behaviour snippet: UFortQuestManager::IsMainQuest (body only).
// Original address: 0x1424470e0 (thunk 0x142ab9a40)
// Offset evidence: Quest+0x88 = UFortPersistableItem::ItemDefinition (reflected Object). Byte at ItemDefinition+0x388 is unreflected on the declared pointee, so it is read raw (uint8).
// Result is true only when that byte is 0 (asm rejects 6 and any nonzero value); null Quest or definition returns false.
// CONFIDENCE: MEDIUM
if (Quest == nullptr || Quest->ItemDefinition == nullptr)
{
    return false;
}
const uint8 DefinitionState = *(reinterpret_cast<const uint8*>(Quest->ItemDefinition) + 0x388);
return DefinitionState == 0;
