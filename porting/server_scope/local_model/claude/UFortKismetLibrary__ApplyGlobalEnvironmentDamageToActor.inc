// CONFIDENCE: MEDIUM. Original exec 0x1429667b0; cores 0x141ee5740 then 0x14161d980.
// Routes environment damage through the global subsystem. Nothing happens without a target or subsystem.
if (!IsValid(DamageTargetActor))
{
	return;
}

UFortEnvironmentDamageSubsystem* Subsystem = GetGlobalEnvironmentDamageSubsystem();
if (!IsValid(Subsystem))
{
	return;
}

IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(DamageTargetActor);
Subsystem->ApplyEnvironmentDamage(AbilityInterface, EnvironmentDamage, AdditionalContextTags, EnvironmentDamageGE);
