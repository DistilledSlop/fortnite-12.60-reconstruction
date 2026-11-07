// Behaviour snippet: UFortQuestMapEventCallout::GetRequiredQuestForCurrentEvent (body only).
// Original body address: 0x1450e9d00 (thunk 0x1452b6230, group B leaf).
// Offset evidence: int32 index at +0x514 (NativeRecoveredInt_0x514, new member_field); TArray<int32> at +0x570 (Data pointer NativeRecoveredPointer_0x570, new member_field; Num NativeRecoveredInt_0x578); QuestMapData Object at +0x508 (reflected); QuestMapData.EventQuestMapDataAssetList (reflected TArray at +0x38, Num at +0x40); asset +0xe0 = RequiredCompletedQuest (reflected); fallback DefaultRequiredCompletedQuest at +0x4f0 (reflected).
// Asm: a null or out-of-range index/slot/asset returns null; otherwise returns the asset's RequiredCompletedQuest when non-null, else the DefaultRequiredCompletedQuest.
// CONFIDENCE: MEDIUM
const int32 EventIndex = NativeRecoveredInt_0x514;
if (EventIndex < 0 || EventIndex >= NativeRecoveredInt_0x578)
{
	return nullptr;
}
const int32 Slot = reinterpret_cast<const int32*>(NativeRecoveredPointer_0x570)[EventIndex];
UFortQuestMapData* MapData = QuestMapData;
if (MapData == nullptr || Slot < 0 || Slot >= MapData->EventQuestMapDataAssetList.Num())
{
	return nullptr;
}
UFortEventQuestMapDataAsset* Asset = MapData->EventQuestMapDataAssetList[Slot];
if (Asset == nullptr)
{
	return nullptr;
}
if (Asset->RequiredCompletedQuest != nullptr)
{
	return Asset->RequiredCompletedQuest;
}
return DefaultRequiredCompletedQuest;
