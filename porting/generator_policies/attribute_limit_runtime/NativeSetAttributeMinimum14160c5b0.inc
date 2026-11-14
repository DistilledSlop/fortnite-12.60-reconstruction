// Original address: 0x14160c5b0.
// CONFIDENCE: HIGH
if (!FGameplayAttribute::IsGameplayAttributeDataProperty(Attribute.GetUProperty()))
    return;
const UAttributeSet* Set = GetAttributeSubobject(Attribute.GetAttributeSetClass());
FGameplayAttributeData* Data = Attribute.GetGameplayAttributeData(const_cast<UAttributeSet*>(Set));
if (Data)
    static_cast<FFortGameplayAttributeData*>(Data)->NativeSetMinimum141bc4cc0(Value);
