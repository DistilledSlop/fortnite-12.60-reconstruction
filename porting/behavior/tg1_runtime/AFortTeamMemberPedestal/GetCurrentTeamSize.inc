// Original body 0x144e329e0 (thunk 0x1452ffb00). Evidence: weak pointer at this+0x3cc (no reflected member covers it, raw access); helper 0x140f930b0 NativeGetOuterUnlessDefault; int32 load at returned object +0x38 (pointee type not reflected, raw access). Invalid weak returns 0.
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const TWeakObjectPtr<UObject>* Weak = reinterpret_cast<const TWeakObjectPtr<UObject>*>(Raw + 0x3cc);
if (!Weak->IsValid())
{
	return 0;
}
UObject* OuterObj = AFortTeamMemberPedestal::NativeGetOuterUnlessDefault140f930b0(Weak->Get());
const uint8* OuterRaw = reinterpret_cast<const uint8*>(OuterObj);
return *reinterpret_cast<const int32*>(OuterRaw + 0x38);
