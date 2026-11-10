// CONFIDENCE: MEDIUM. Authority-only. Registers this spawner in the game state's special-actor registry under the key "<ItemName>_<ActorName>" and stores that key in SpecialActorID when the registry accepts the record.
// Original addresses: exec thunk 0x14274bd20; direct helper 0x141c18510 (its logic is inlined below).
if (Role != ROLE_Authority || !SpecialActorID.IsNone() || ItemDefinition == nullptr)
{
    return;
}
UWorld* World = GetWorld();
AFortGameStateAthena* GameState = GetFortGameStateAthena(World);
if (GameState == nullptr || GameState->SpecialActorRegistry == nullptr)
{
    return;
}
const FString RegistryKeyText = FString::Printf(TEXT("%s_%s"), *ItemDefinition->GetFName().ToString(), *GetFName().ToString());
const FName RegistryKey(*RegistryKeyText, FNAME_Add);

FSpecialActorRecord Record;
Record.Actor = this;
Record.Key = RegistryKey;
Record.ItemDefinitionName = ItemDefinition->GetFName();
// The helper also copies the item definition's remaining data members (text block, two int pairs, one array) into the record; those member names are not resolved in the dump.
Record.ItemDefinitionData = ItemDefinition->ItemRegistryData;

if (GameState->SpecialActorRegistry->Add(Record) != INDEX_NONE)
{
    SpecialActorID = RegistryKey;
}
