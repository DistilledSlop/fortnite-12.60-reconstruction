// Behaviour snippet: UObjectInteractionBehavior::GetFortCreativeMoveTool (body only; spliced into the generated member function).
// Original address: 0x14266d050
// Offset evidence: object pointer at +0xa0 (NativeRecoveredUint64_0xa0; the gap after the UActorComponent array that ends at 0xa0). The pointer is returned when its ClassPrivate IsChildOf AFortCreativeMoveTool (UObject::IsA fast path), otherwise null.
// CONFIDENCE: MEDIUM
UObject* Owner = reinterpret_cast<UObject*>(NativeRecoveredUint64_0xa0);
if (Owner == nullptr || !Owner->IsA(AFortCreativeMoveTool::StaticClass()))
{
	return nullptr;
}
return reinterpret_cast<AFortCreativeMoveTool*>(Owner);
