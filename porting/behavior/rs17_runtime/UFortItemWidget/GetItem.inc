// Original body address: 0x144d7a3b0 (thunk 0x14527c8e0)
// Offset evidence: weak item pointer at object +0x82c (unreflected gap between LastCooldownPct +0x824 and QuantityOverride +0x834, raw access). IsValid (0x142ee7880) false -> XOR EAX,EAX; true -> Get (0x142ee5dd0) tail call.
// CONFIDENCE: HIGH
const TWeakObjectPtr<UObject>* ItemWeak = reinterpret_cast<const TWeakObjectPtr<UObject>*>(reinterpret_cast<const uint8*>(this) + 0x82c);
if (!ItemWeak->IsValid())
{
    return nullptr;
}
return static_cast<UFortItem*>(ItemWeak->Get());
