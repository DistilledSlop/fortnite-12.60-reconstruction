// Original body 0x144e485a0 (thunk 0x145300010). Evidence: weak at this+0x3cc (raw, unreflected); first Get yields owner whose weak at +0x30 (raw, pointee unreflected); tail-jump to NativeHelper142261600 on the second Get result; null paths return false.
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const TWeakObjectPtr<UObject>* Weak = reinterpret_cast<const TWeakObjectPtr<UObject>*>(Raw + 0x3cc);
if (!Weak->IsValid())
{
	return false;
}
UObject* Owner = Weak->Get();
const uint8* OwnerRaw = reinterpret_cast<const uint8*>(Owner);
const TWeakObjectPtr<UObject>* InnerWeak = reinterpret_cast<const TWeakObjectPtr<UObject>*>(OwnerRaw + 0x30);
UObject* TargetObj = InnerWeak->Get();
if (!TargetObj)
{
	return false;
}
return AFortTeamMemberPedestal::NativeHelper142261600(TargetObj);
