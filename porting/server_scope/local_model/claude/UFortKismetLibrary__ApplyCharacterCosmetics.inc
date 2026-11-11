// CONFIDENCE: LOW. Original exec 0x142965ca0; cores 0x141ee4ab0 then 0x141d9ef80.
// Fails (bSuccess false) unless the player state is authoritative, has parts to apply, an ASC, an avatar and a cosmetics component.
bSuccess = false;
if (!IsValid(PlayerState) || CharacterParts.Num() <= 0 || PlayerState->GetLocalRole() != ROLE_Authority)
{
	return;
}

UFortAbilitySystemComponent* AbilitySystem = PlayerState->GetFortAbilitySystemComponent();
if (!IsValid(AbilitySystem))
{
	return;
}

AActor* Avatar = AbilitySystem->GetAvatarActor();
UFortCosmeticsComponent* Cosmetics = PlayerState->CosmeticsComponent;
if (!IsValid(Cosmetics) || !IsValid(Avatar))
{
	return;
}

bool bFlagA = false;
bool bFlagB = false;
AbilitySystem->GetCosmeticApplyFlags(bFlagA, bFlagB);
const FGameplayAbilityActorInfo ActorInfo = AbilitySystem->MakeActorInfo();
bSuccess = Cosmetics->ApplyCharacterParts(WorldContextObject, ActorInfo, Avatar, CharacterParts, bFlagA, bFlagB);
