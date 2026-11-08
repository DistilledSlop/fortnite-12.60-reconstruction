// Original body address: 0x1407c5240 (helper, no thunk)
// Offset evidence: +0x260 UCommonButton::Style (reflected Class) read as UClass*; the class default object is UClass +0x130 (ClassDefaultObject), read here through GetDefaultObject(false), which does not create one.
// Asm: null/IsChildOf(UCommonButtonStyle) check on the style class, CDO fetch, then IsChildOf check on the CDO's class. The asm repeats the first class check on identical operands; the repeat is elided.
// CONFIDENCE: MEDIUM
UClass* ButtonStyleClass = *reinterpret_cast<UClass* const*>(reinterpret_cast<const uint8*>(Button) + 0x260);
if (ButtonStyleClass == nullptr)
{
	return nullptr;
}
if (!ButtonStyleClass->IsChildOf(UCommonButtonStyle::StaticClass()))
{
	return nullptr;
}
UObject* ButtonStyleDefault = ButtonStyleClass->GetDefaultObject(false);
if (ButtonStyleDefault == nullptr)
{
	return nullptr;
}
if (!ButtonStyleDefault->GetClass()->IsChildOf(UCommonButtonStyle::StaticClass()))
{
	return nullptr;
}
return ButtonStyleDefault;
