// CONFIDENCE: MEDIUM
// Native 0x14296a090 -> 0x141eec730: the set is granted only when the actor's ability-system interface (object and
// interface pointer) and the ability set are all present; otherwise an empty handle is returned. The native give
// step resolves the ability system component through the interface (virtual slot 0x10).
FFortAbilitySetHandle Handle;
if (AbilitySystemInterfaceActor.GetObject() && AbilitySystemInterfaceActor.GetInterface() && AbilitySet)
{
    Handle = AbilitySet->GiveToAbilitySystem(AbilitySystemInterfaceActor.GetInterface(), OverrideSourceObject);
}
return Handle;
