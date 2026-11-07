// Behaviour snippet: UAIHotSpotSlotGenerator::GetHotSpot (body only).
// Original body address: 0x14165bce0 (thunk 0x1427109e0), receiver delta 0. Same body as UAIHotSpotSlot::GetHotSpot.
// Evidence: this+0x20 is loaded as an object pointer. The class has no reflected properties, so the offset is read raw from the object address. The body reads the object's ClassPrivate (+0x10) and runs the class-tree IsChildOf check against AAIHotSpot::StaticClass() (callee 0x14270e1a0); on a match it returns the object, otherwise null.
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
UObject* Owner = *reinterpret_cast<UObject* const*>(Raw + 0x20);
if (Owner == nullptr || !Owner->IsA(AAIHotSpot::StaticClass()))
{
    return nullptr;
}
return static_cast<AAIHotSpot*>(Owner);
