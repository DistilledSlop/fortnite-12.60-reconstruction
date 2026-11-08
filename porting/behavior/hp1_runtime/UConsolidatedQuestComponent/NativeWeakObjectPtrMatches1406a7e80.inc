// Helper 0x1406a7e80, host UConsolidatedQuestComponent (static).
// Evidence: CALL 0x142ee3b30 (TWeakObjectPtr assign from UObject*) into a temporary, then compares index (+0) and serial (+4) with SelfWeak.
// Equal index and serial -> true. Otherwise false if SelfWeak is valid, false if the temporary is valid, else true.
// Callers: UCommonPopupMenu::SetContextProvider (weak member +0x41c), ABuildingWall::OnNearDoorTriggerTouched.
// CONFIDENCE: HIGH
const TWeakObjectPtr<UObject> OtherWeak(OtherObject);
const int32* SelfWords = reinterpret_cast<const int32*>(SelfWeak);
const int32* OtherWords = reinterpret_cast<const int32*>(&OtherWeak);
if (SelfWords[0] == OtherWords[0] && SelfWords[1] == OtherWords[1])
{
    return true;
}
if (SelfWeak->IsValid())
{
    return false;
}
if (OtherWeak.IsValid())
{
    return false;
}
return true;
