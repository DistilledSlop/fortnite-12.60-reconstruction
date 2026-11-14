// CONFIDENCE: HIGH
// ABuildingLayoutRequirement* NativeFindLayout141c78520(uint32 UID) const
const TWeakObjectPtr<ABuildingLayoutRequirement>* Weak = NativeLayoutByUID_0x2b8.Find(UID);
ABuildingLayoutRequirement* Actor = Weak ? Weak->Get() : nullptr;
return Actor && !Actor->IsPendingKill() && !Actor->bDestroyed ? Actor : nullptr;
