// Behaviour snippet: UAIHotSpotSlot::GetHotSpot (body only).
// Original body address: 0x14165bce0 (thunk 0x1427109e0), receiver delta 0.
// Evidence: this+0x20 is loaded as an object pointer (member NativeRecoveredPtr_0x20; no reflected property covers 0x20 and the class's lowest own offset is 0x0). The body reads the object's ClassPrivate (+0x10) and runs the class-tree IsChildOf check against AAIHotSpot::StaticClass() (callee 0x14270e1a0); on a match it returns the object, otherwise null.
// CONFIDENCE: MEDIUM
UObject* Owner = reinterpret_cast<UObject*>(NativeRecoveredPtr_0x20);
if (Owner == nullptr || !Owner->IsA(AAIHotSpot::StaticClass()))
{
    return nullptr;
}
return static_cast<AAIHotSpot*>(Owner);
