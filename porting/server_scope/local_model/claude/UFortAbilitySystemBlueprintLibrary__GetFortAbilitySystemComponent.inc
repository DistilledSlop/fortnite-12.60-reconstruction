// CONFIDENCE: HIGH. Returns the Fort ability system component the actor exposes through IFortAbilitySystemInterface, or null. The Blueprint path does not warn for non-implementers (helper 0x1415fa910 called with bWarnIfMissing = false).
// Original addresses: exec thunk 0x1427813d0; body 0x141df7430 -> helper 0x1415fa910.
if (!Actor)
{
	return nullptr;
}
IFortAbilitySystemInterface* AbilityActor = Cast<IFortAbilitySystemInterface>(Actor);
if (!AbilityActor)
{
	return nullptr;
}
return AbilityActor->NativeGetFortAbilitySystemComponent();
