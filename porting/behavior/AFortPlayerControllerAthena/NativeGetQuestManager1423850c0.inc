// CONFIDENCE: HIGH
// 0x1423850c0 tail-calls the registered player info's selector when present.
UFortRegisteredPlayerInfo* PlayerInfo = MyPlayerInfo;
if (!PlayerInfo)
{
    return nullptr;
}
if (Mode == 2)
{
    if (UFortGameInstance* GameInstance = Cast<UFortGameInstance>(PlayerInfo->GetOuter()))
    {
        Mode = GameInstance->NativeGetSubGame141e5f160();
    }
}
return PlayerInfo->QuestManagers[Mode == 1 ? 1 : 0];
