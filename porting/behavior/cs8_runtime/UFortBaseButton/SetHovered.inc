// Body 0x144d95470 (thunk 0x145205970): receiver weak ptr at object+0x7d8 (gap, recovered as NativeRecoveredUInt64_0x7d8); pointee field [weak+0x430] (pointee class not reflected)
// Evidence: CALL 0x142ee5dd0 = TWeakObjectPtr::Get on RCX=this+0x7d8; JZ returns on null; [RAX+0x430] loaded as TargetObj; tail-call 0x1423af600 with DL=bool as uint8
// CONFIDENCE: MEDIUM
const UObject* const WeakTarget = reinterpret_cast<TWeakObjectPtr<UObject>*>(&NativeRecoveredUInt64_0x7d8)->Get();
if (WeakTarget == nullptr)
{
	return;
}
UObject* const TargetObj = *reinterpret_cast<UObject* const*>(reinterpret_cast<const uint8*>(WeakTarget) + 0x430);
if (TargetObj == nullptr)
{
	return;
}
UFortBaseButton::NativeSetHoverFlagBit1423af600(TargetObj, static_cast<uint8>(InHovered));
