// Original address: 0x14160c540.
// CONFIDENCE: HIGH
if (!FGameplayAttribute::IsGameplayAttributeDataProperty(Attribute.GetUProperty()))
    return;
const UAttributeSet* Set = GetAttributeSubobject(Attribute.GetAttributeSetClass());
FGameplayAttributeData* Data = Attribute.GetGameplayAttributeData(const_cast<UAttributeSet*>(Set));
if (Data)
    static_cast<FFortGameplayAttributeData*>(Data)->NativeSetMaximum141bc4c50(Value);
