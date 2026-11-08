// Original body address: 0x144bebfd0 (thunk 0x145338000)
// Offset evidence: LEA RBX,[RCX+0x2b4] takes the address of an 8-byte TWeakObjectPtr at object offset 0x2b4. lookup_offset.py finds no reflected member covering 0x2b4 in this class (no own reflected members) or its ancestors (last reflected UPostGamePlacementOverlay member 0x298), so raw access. Helper result [RAX+0x4] is read from an untyped uint8* (int32 at +0x4).
// CONFIDENCE: MEDIUM
static_assert(sizeof(UInfiltrationStatusComponent) > 0, "UInfiltrationStatusComponent must be complete for NativeHelper141b03e10");
const uint8* ObjectBytes = reinterpret_cast<const uint8*>(this);
const TWeakObjectPtr<UObject>* IntelWeak = reinterpret_cast<const TWeakObjectPtr<UObject>*>(ObjectBytes + 0x2b4);
if (!IntelWeak->IsValid())
{
    return 0;
}
const uint8* HelperResult = UInfiltrationStatusComponent::NativeHelper141b03e10(IntelWeak->Get());
return *reinterpret_cast<const int32*>(HelperResult + 0x4);
